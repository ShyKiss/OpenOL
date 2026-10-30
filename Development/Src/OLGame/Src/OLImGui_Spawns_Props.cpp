/*=============================================================================
    OLImGui_Spawns_Props.cpp — Props sub-tab: browse and place StaticMesh actors.

    On first open the Props tab spawns a background thread (FPropsScanThread) that
    enumerates all .upk files under CookedPCConsole and CookedPCConsoleDLC and scans
    each one's ExportMap for StaticMesh exports — no ULinkerLoad, no full package load,
    no UObject/FName/GWorld access, so it's safe off the game thread. The game thread
    just polls for completion and copies the results out.

    Duplicates (same MeshName across packages) are silently dropped; the first
    occurrence wins.  After scanning the list is sorted by MeshName.

    Previews use a dedicated FOLPreviewInstance (GPropsPreview) with an LRU
    cache of up to PROPS_LRU_SIZE loaded UStaticMesh pointers.
=============================================================================*/
#include "OLImGui_Tabs.h"

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static const int PROPS_LRU_SIZE     = 16;
static const int MAX_PLACED_PROPS   = 512;

// ---------------------------------------------------------------------------
// Data types
// ---------------------------------------------------------------------------

// Prop index entry — only strings, no UObject pointers.
struct FPropEntry
{
    char PackageName[128];
    char MeshPackage[128];
    char MeshName[128];
};

// Snapshot of a placed prop actor (tag=CustomSpawnedProp), rebuilt each tick.
struct FPlacedPropEntry
{
    char  Name[64];        // actor name (display + delete key)
    char  MeshName[128];   // StaticMesh asset name
    char  PackageName[128];// source package
    bool  bHasCollision;
    FName ActorFName;
};

// LRU mesh cache entry (see "LRU mesh cache" section below).
struct FPropsLRUEntry
{
    char         PackageName[128];
    char         MeshName[128];
    UStaticMesh* Mesh;
    int          Ticket;
};

// ---------------------------------------------------------------------------
// Package scanner — reads compressed/uncompressed .upk without ULinkerLoad.
// Self-contained: no globals, no UObject/FName/GWorld access. Safe to call
// from the background scan thread further below.
// ---------------------------------------------------------------------------

// Dedup key for the scanner's mesh-name dedup pass: an FNV-1a hash of the raw ANSI mesh
// name, computed directly off the bytes read from the .upk name table. Avoids allocating
// an FString and converting ANSI->TCHAR (ANSI_TO_TCHAR + FString's own hash-over-TCHAR)
// for every StaticMesh export across every scanned package - the scan can hit tens of
// thousands of exports, and that conversion/allocation was a real chunk of the per-package
// cost. Cross-package dedup by mesh name still works: the hash only depends on the name
// bytes, not on anything package-specific (export index would NOT work here - it's only
// meaningful within the .upk it came from).
static DWORD HashMeshNameAnsi(const char* Name)
{
    // FNV-1a, 32-bit.
    DWORD Hash = 2166136261u;
    for (const char* p = Name; *p; ++p)
    {
        Hash ^= (BYTE)*p;
        Hash *= 16777619u;
    }
    return Hash;
}

// Decompresses chunks of a compressed .upk on demand and keeps a small LRU cache
// of the decompressed results, so seeking backward (e.g. ExportMap then back into
// NameMap) doesn't re-decompress chunks already visited. Knows nothing about the
// archive/cursor wrapping it — just "given a compressed chunk table + file, get me
// the decompressed bytes at file offset X".
class FCompressedChunkCache
{
public:
    FCompressedChunkCache(FArchive* InF, const TArray<FCompressedChunk>& InChunks, ECompressionFlags InFlags)
        : F(InF), ComprFlags(InFlags), Chunks(InChunks), LRUClock(0) {}

    ~FCompressedChunkCache()
    {
        for (INT i = 0; i < CachedChunks.Num(); ++i) appFree(CachedChunks(i).Buf);
    }

    // Returns a pointer to the decompressed bytes covering UncompPos, and writes
    // that chunk's [OutChunkOff, OutChunkOff+OutChunkSize) range. NULL on error.
    const BYTE* GetChunkAt(INT UncompPos, INT& OutChunkOff, INT& OutChunkSize, UBOOL& bOutError)
    {
        // Check the LRU cache first.
        for (INT i = 0; i < CachedChunks.Num(); ++i)
        {
            FCachedChunk& C = CachedChunks(i);
            if (UncompPos >= C.Off && UncompPos < C.Off + C.Size)
            {
                C.LastUsed = ++LRUClock;
                OutChunkOff = C.Off; OutChunkSize = C.Size;
                return C.Buf;
            }
        }

        // Miss: find and decompress the chunk covering UncompPos.
        for (INT ci = 0; ci < Chunks.Num(); ++ci)
        {
            const FCompressedChunk& C = Chunks(ci);
            if (UncompPos >= C.UncompressedOffset &&
                UncompPos <  C.UncompressedOffset + C.UncompressedSize)
            {
                BYTE* NewBuf = (BYTE*)appMalloc(C.UncompressedSize);
                F->Seek(C.CompressedOffset);
                try {
                    F->SerializeCompressed(NewBuf, C.UncompressedSize, ComprFlags);
                } catch (...) { appFree(NewBuf); bOutError = TRUE; return NULL; }
                if (F->IsError()) { appFree(NewBuf); bOutError = TRUE; return NULL; }

                if (CachedChunks.Num() >= MAX_CACHED_CHUNKS)
                {
                    // Evict least-recently-used slot.
                    INT LruIdx = 0;
                    for (INT i = 1; i < CachedChunks.Num(); ++i)
                        if (CachedChunks(i).LastUsed < CachedChunks(LruIdx).LastUsed) LruIdx = i;
                    appFree(CachedChunks(LruIdx).Buf);
                    CachedChunks.Remove(LruIdx);
                }

                FCachedChunk Entry;
                Entry.Buf = NewBuf; Entry.Off = C.UncompressedOffset; Entry.Size = C.UncompressedSize;
                Entry.LastUsed = ++LRUClock;
                CachedChunks.AddItem(Entry);

                OutChunkOff = Entry.Off; OutChunkSize = Entry.Size;
                return NewBuf;
            }
        }
        bOutError = TRUE;
        return NULL;
    }

private:
    // Small LRU cache: at most MAX_CACHED_CHUNKS decompressed chunks kept
    // around at once, evicting the least-recently-used on overflow.
    struct FCachedChunk
    {
        BYTE* Buf;
        INT   Off;
        INT   Size;
        DWORD LastUsed;
    };
    enum { MAX_CACHED_CHUNKS = 8 };

    FArchive*                 F; // not owned
    ECompressionFlags         ComprFlags;
    TArray<FCompressedChunk>  Chunks;
    TArray<FCachedChunk>      CachedChunks;
    DWORD                     LRUClock;
};

// Thin safe wrapper around FArchive* — guards Serialize/Seek against out-of-bounds
// without loading the entire file into memory.
// For compressed packages, decompression/caching is delegated to FCompressedChunkCache;
// this class just tracks the read cursor and the "currently resident" chunk.
class FSafePkgAr : public FArchive
{
public:
    // Flat in-memory buffer (takes ownership of Buf).
    FSafePkgAr(BYTE* Buf, INT InSize)
        : F(NULL), ChunkCache(NULL), FileSize(InSize), CurPos(0), bCompressed(false)
        , ChunkBuf(Buf), ChunkBufOff(0), ChunkBufSize(InSize)
    { ArIsLoading = TRUE; ArIsPersistent = TRUE; }

    // Compressed: reads + decompresses chunks on demand via ChunkCache.
    FSafePkgAr(FArchive* InF, INT InUncompSize,
               const TArray<FCompressedChunk>& InChunks, ECompressionFlags InFlags)
        : F(InF), FileSize(InUncompSize), CurPos(0), bCompressed(true)
        , ChunkBuf(NULL), ChunkBufOff(-1), ChunkBufSize(0)
    {
        ArIsLoading = TRUE; ArIsPersistent = TRUE;
        ChunkCache = new FCompressedChunkCache(InF, InChunks, InFlags);
    }

    ~FSafePkgAr()
    {
        if (bCompressed) delete ChunkCache;
        else             appFree(ChunkBuf); // flat buffer: ChunkBuf owns it directly
        if (F) delete F;
    }

    void Serialize(void* Dst, INT Num)
    {
        if (Num <= 0 || ArIsError) return;
        if (CurPos + Num > FileSize) { ArIsError = TRUE; return; }
        if (!bCompressed)
        {
            // Flat buffer: ChunkBuf holds the entire addressable region.
            if (CurPos + Num > ChunkBufSize) { ArIsError = TRUE; return; }
            appMemcpy(Dst, ChunkBuf + CurPos, Num);
            CurPos += Num;
            return;
        }
        // Compressed: may span chunk boundaries.
        BYTE* Out = (BYTE*)Dst;
        INT Remaining = Num;
        while (Remaining > 0 && !ArIsError)
        {
            if (!EnsureChunkAt(CurPos)) return;
            INT OffInChunk = CurPos - ChunkBufOff;
            INT Available  = ChunkBufSize - OffInChunk;
            INT Copy       = Remaining < Available ? Remaining : Available;
            appMemcpy(Out, ChunkBuf + OffInChunk, Copy);
            Out       += Copy;
            CurPos    += Copy;
            Remaining -= Copy;
        }
    }

    void Seek(INT InPos)
    {
        if (InPos < 0 || InPos > FileSize) { ArIsError = TRUE; return; }
        CurPos = InPos;
    }

    void SetError()  { ArIsError = TRUE; }
    INT  Tell()      { return CurPos; }
    INT  TotalSize() { return FileSize; }
    virtual FString GetArchiveName() const { return TEXT("FSafePkgAr"); }

private:
    // Refreshes ChunkBuf/ChunkBufOff/ChunkBufSize to cover UncompPos. Returns
    // false (and sets ArIsError) on failure.
    bool EnsureChunkAt(INT UncompPos)
    {
        if (ChunkBuf && ChunkBufOff >= 0 &&
            UncompPos >= ChunkBufOff && UncompPos < ChunkBufOff + ChunkBufSize)
            return true;

        UBOOL bError = FALSE;
        const BYTE* Buf = ChunkCache->GetChunkAt(UncompPos, ChunkBufOff, ChunkBufSize, bError);
        if (!Buf) { ArIsError = TRUE; return false; }
        ChunkBuf = const_cast<BYTE*>(Buf);
        return true;
    }

    FArchive*               F;          // owned; underlying file (compressed path only)
    FCompressedChunkCache*  ChunkCache; // owned; compressed path only
    INT                     FileSize;
    INT                     CurPos;
    bool                    bCompressed;
    BYTE*                   ChunkBuf;     // points into ChunkCache (or flat buffer); not owned here
    INT                     ChunkBufOff;
    INT                     ChunkBufSize;
};

// Open a .upk for scanning — reads only NameMap+ImportMap+ExportMap into one buffer.
// For uncompressed packages this is a single contiguous read covering just the header
// tables, which is far smaller than the full file (no bulk data, no texture mips, etc.).
static FSafePkgAr* OpenPackageForScan(const FString& FullPath, FPackageFileSummary& OutSummary)
{
    FArchive* F = GFileManager->CreateFileReader(*FullPath);
    if (!F) return NULL;

    *F << OutSummary;
    if (F->IsError()) { delete F; return NULL; }

    const INT FileSize = F->TotalSize();

    if (OutSummary.CompressedChunks.Num() > 0)
    {
        const FCompressedChunk& Last = OutSummary.CompressedChunks.Last();
        const INT UncompSize = Last.UncompressedOffset + Last.UncompressedSize;
        return new FSafePkgAr(F, UncompSize, OutSummary.CompressedChunks,
            (ECompressionFlags)OutSummary.CompressionFlags);
    }

    // Uncompressed: read only the tables we need in one contiguous block.
    // Tables are almost always laid out as: NameMap → ImportMap → ExportMap.
    // Compute the bounding region [Lo, Hi) that covers all three sections.
    // Each export's serial data (bulk meshes/textures) lives past ExportOffset+ExportCount*sizeof(entry)
    // and is NOT needed for scanning — skip it.
    const INT ExportEntrySize = 256; // conservative upper bound per export entry
    INT Lo = FileSize, Hi = 0;
    auto Expand = [&](INT Off, INT Bytes) {
        if (Off > 0 && Bytes > 0) {
            if (Off     < Lo) Lo = Off;
            if (Off+Bytes > Hi) Hi = Off+Bytes;
        }
    };
    // Name entries are variable-length; read up to NameOffset + NameCount*NAME_SIZE as upper bound.
    Expand(OutSummary.NameOffset,   OutSummary.NameCount   * (NAME_SIZE + 8));
    Expand(OutSummary.ImportOffset, OutSummary.ImportCount * 28); // FObjectImport ~28 bytes
    Expand(OutSummary.ExportOffset, OutSummary.ExportCount * ExportEntrySize);
    Lo = Max(Lo, 0);
    Hi = Min(Hi, FileSize);
    if (Lo >= Hi) { delete F; return NULL; }

    const INT BufSize = Hi - Lo;
    BYTE* Buf = (BYTE*)appMalloc(BufSize);
    F->Seek(Lo); F->Serialize(Buf, BufSize);
    const bool bOK = !F->IsError();
    delete F;
    if (!bOK) { appFree(Buf); return NULL; }

    // Wrap in a FSafePkgAr backed by a flat in-memory buffer at logical offset Lo.
    // We construct with the full FileSize so that offset validation passes, but the
    // buffer only covers [Lo, Hi). Accesses outside that range set ArIsError.
    // Use a small helper: give FSafePkgAr the buffer shifted so index 0 == file offset 0.
    // Easiest: allocate a zero-padded buffer of size Hi and copy Buf into it at offset Lo.
    // (Hi is at most a few MB — the header tables, not the full file.)
    BYTE* FullBuf = (BYTE*)appMalloc(Hi);
    appMemzero(FullBuf, Lo);           // zero the region before Lo (never accessed)
    appMemcpy(FullBuf + Lo, Buf, BufSize);
    appFree(Buf);
    return new FSafePkgAr(FullBuf, Hi); // FSafePkgAr owns FullBuf
}

// No-FName name table — avoids touching the global FName table during scanning.
// Stores byte offsets into Buf (not pointers) so realloc doesn't invalidate anything.
struct FRawNames
{
    TArray<INT>  Offs; // byte offset of each name in Buf
    TArray<char> Buf;  // pooled string storage

    void Add(const ANSICHAR* Str, INT Len)
    {
        INT Off = Buf.Num();
        Offs.AddItem(Off);
        Buf.Add(Len + 1);
        appMemcpy(&Buf(Off), Str, Len);
        Buf(Off + Len) = '\0';
    }

    const char* Get(INT Idx) const
    {
        if (Idx < 0 || Idx >= Offs.Num() || Offs(Idx) < 0) return "";
        return &Buf(Offs(Idx));
    }

    INT Num() const { return Offs.Num(); }
};

// Scans one package and appends every StaticMesh export found to OutResults (no dedup —
// callers dedup once across the full result set). Pure file I/O + local parsing, no
// UObject/FName/GWorld access — safe to call from a worker thread.
static void PropsScanPackage(const FString& FullPath, const char* PkgBuf, TArray<FPropEntry>& OutResults)
{
    FPackageFileSummary Sum;
    FSafePkgAr* Ar = OpenPackageForScan(FullPath, Sum);
    if (!Ar) return;

    const INT Ver      = Sum.GetFileVersion();
    const INT FileSize = Ar->TotalSize();

    auto ValidOff = [&](INT Off, INT Count) -> UBOOL {
        return Off > 0 && Off < FileSize && Count > 0 && Count < 500000;
    };
    if (!ValidOff(Sum.NameOffset, Sum.NameCount) ||
        !ValidOff(Sum.ExportOffset, Sum.ExportCount) ||
        Sum.ImportCount < 0 || Sum.ImportCount > 500000)
    { delete Ar; return; }

    Ar->SetVer(Ver);
    Ar->SetLicenseeVer(Sum.GetFileVersionLicensee());

    // Read a raw (NameIdx, Number) pair; only the name index is kept — resolved later.
    auto ReadNIdx = [&]() -> INT {
        INT Idx = 0, Num = 0;
        Ar->Serialize(&Idx, 4); Ar->Serialize(&Num, 4);
        if (Ar->IsError() || Idx < 0 || Idx >= Sum.NameCount) return -1;
        return Idx;
    };

    // ImportMap first — only ClassName and ObjectName name-indices + OuterIndex.
    // Every import's ObjectName is needed (to find which one is named "StaticMesh"),
    // so no filtering is possible here; this table is small relative to NameMap.
    struct FRI { INT ClsIdx; INT ObjIdx; INT Outer; };
    TArray<FRI> Imports;
    if (Sum.ImportCount > 0 && Sum.ImportOffset > 0)
    {
        Ar->Seek(Sum.ImportOffset);
        Imports.Empty(Sum.ImportCount);
        for (INT i = 0; i < Sum.ImportCount && !Ar->IsError(); ++i)
        {
            FRI R;
            INT ClsPkg = ReadNIdx();   // ClassPackage
            R.ClsIdx   = ReadNIdx();   // ClassName
            Ar->Serialize(&R.Outer, 4);
            R.ObjIdx   = ReadNIdx();   // ObjectName
            if (!Ar->IsError()) Imports.AddItem(R);
            (void)ClsPkg;
        }
    }
    if (Ar->IsError()) { delete Ar; return; }

    // ExportMap — ClassIndex, ObjectName name-index, SerialOffset, SerialSize.
    struct FRE { INT ClsIdx; INT ObjIdx; INT SerOff; INT SerSize; };
    TArray<FRE> Exports;
    if (Sum.ExportCount > 0 && Sum.ExportOffset > 0)
    {
        Ar->Seek(Sum.ExportOffset);
        Exports.Empty(Sum.ExportCount);
        for (INT i = 0; i < Sum.ExportCount && !Ar->IsError(); ++i)
        {
            FRE E2;
            Ar->Serialize(&E2.ClsIdx, 4);     // ClassIndex
            INT Super = 0; Ar->Serialize(&Super, 4);
            INT Outer = 0; Ar->Serialize(&Outer, 4);
            E2.ObjIdx = ReadNIdx();            // ObjectName
            INT Arch  = 0; Ar->Serialize(&Arch, 4);
            QWORD Flags = 0; Ar->Serialize(&Flags, 8);
            Ar->Serialize(&E2.SerSize, 4); Ar->Serialize(&E2.SerOff, 4);
            if (Ver < VER_REMOVED_COMPONENT_MAP)
            {
                INT MC = 0; Ar->Serialize(&MC, 4);
                if (MC > 0 && MC < 10000) Ar->Seek(Ar->Tell() + MC * 12);
            }
            INT XF = 0; Ar->Serialize(&XF, 4);
            INT GC = 0; Ar->Serialize(&GC, 4);
            if (GC > 0 && GC < 10000) Ar->Seek(Ar->Tell() + GC * 4);
            Ar->Seek(Ar->Tell() + 16 + 4); // PackageGuid + PackageFlags
            if (!Ar->IsError()) Exports.AddItem(E2);
        }
    }
    if (Ar->IsError()) { delete Ar; return; }

    // Resolve class name-index for an export (Import class ref or Export-as-class ref).
    auto GetClsNameIdx = [&](const FRE& E2) -> INT {
        if (E2.ClsIdx < 0) {
            INT ii = -E2.ClsIdx - 1;
            if (ii < Imports.Num()) return Imports(ii).ObjIdx;
        } else if (E2.ClsIdx > 0) {
            INT ii = E2.ClsIdx - 1;
            if (ii < Exports.Num()) return Exports(ii).ObjIdx;
        }
        return -1;
    };

    // Build the set of NameMap indices we actually need a string for:
    // every import's ObjectName (to spot the "StaticMesh" import), plus every
    // export's own ObjectName (mesh names) and its class name-index. Only these
    // get a string allocated in the pass below — everything else is skipped.
    TMap<INT,BYTE> WantedNameIdx;
    for (INT i = 0; i < Imports.Num(); ++i)
        if (Imports(i).ObjIdx >= 0) WantedNameIdx.Set(Imports(i).ObjIdx, 1);
    for (INT i = 0; i < Exports.Num(); ++i)
    {
        const FRE& E2 = Exports(i);
        if (E2.ObjIdx >= 0) WantedNameIdx.Set(E2.ObjIdx, 1);
        INT ClsNameIdx = GetClsNameIdx(E2);
        if (ClsNameIdx >= 0) WantedNameIdx.Set(ClsNameIdx, 1);
    }

    // NameMap — single sequential pass (every chunk gets decompressed regardless),
    // but only names whose index is in WantedNameIdx get stored in Names.Buf.
    FRawNames Names;
    Names.Offs.AddZeroed(Sum.NameCount);
    for (INT i = 0; i < Sum.NameCount; ++i) Names.Offs(i) = -1; // -1 == not stored
    Names.Buf.Reserve(WantedNameIdx.Num() * 24); // average name ~24 chars
    if (Sum.NameCount > 0 && Sum.NameOffset > 0)
    {
        Ar->Seek(Sum.NameOffset);
        for (INT i = 0; i < Sum.NameCount && !Ar->IsError(); ++i)
        {
            INT StringLen = 0;
            Ar->Serialize(&StringLen, 4);
            if (Ar->IsError()) break;
            const UBOOL bUnicode = (StringLen < 0);
            const INT   AbsLen   = bUnicode ? -StringLen : StringLen;
            if (AbsLen <= 0 || AbsLen > NAME_SIZE) { Ar->SetError(); break; }
            const bool bWanted = WantedNameIdx.Find(i) != NULL;
            ANSICHAR Ansi[NAME_SIZE] = {};
            if (bUnicode)
            {
                for (INT c = 0; c < AbsLen && !Ar->IsError(); ++c)
                { WORD W = 0; Ar->Serialize(&W, 2); if (bWanted && c < NAME_SIZE-1) Ansi[c] = (ANSICHAR)(W & 0x7F); }
            }
            else
            {
                Ar->Serialize(Ansi, AbsLen);
            }
            QWORD DummyFlags = 0; Ar->Serialize(&DummyFlags, 8);
            if (!Ar->IsError() && bWanted)
            {
                INT Len = AbsLen > 0 ? AbsLen - 1 : 0;
                Names.Offs(i) = Names.Buf.Num();
                Names.Buf.Add(Len + 1);
                appMemcpy(&Names.Buf(Names.Offs(i)), Ansi, Len);
                Names.Buf(Names.Offs(i) + Len) = '\0';
            }
        }
    }
    if (Ar->IsError()) { delete Ar; return; }

    // Resolve class name for an export, now that Names is populated.
    auto GetCls = [&](const FRE& E2) -> const char* {
        INT ClsNameIdx = GetClsNameIdx(E2);
        return ClsNameIdx >= 0 ? Names.Get(ClsNameIdx) : "Class";
    };

    // Find all StaticMesh exports.
    for (INT xi = 0; xi < Exports.Num(); ++xi)
    {
        const FRE& E2 = Exports(xi);
        if (appStricmp(GetCls(E2), "StaticMesh") != 0) continue;

        const char* MeshName = Names.Get(E2.ObjIdx);
        if (!MeshName || MeshName[0] == '\0') continue;

        FPropEntry Entry;
        appMemzero(&Entry, sizeof(Entry));
        appStrncpyANSI(Entry.PackageName, PkgBuf,  ARRAY_COUNT(Entry.PackageName));
        appStrncpyANSI(Entry.MeshPackage, PkgBuf,  ARRAY_COUNT(Entry.MeshPackage));
        appStrncpyANSI(Entry.MeshName,    MeshName, ARRAY_COUNT(Entry.MeshName));
        OutResults.AddItem(Entry);
    }

    delete Ar;
}

// Enumerates all .upk files under CookedPCConsole + CookedPCConsoleDLC. Pure
// path/FindFiles work, no UObject access — safe to call from a worker thread.
static void PropsEnumeratePackages(TArray<FString>& OutPackages)
{
    FString CookedPath;
    appGetCookedContentPath(appGetPlatformType(), CookedPath);

    TArray<FString> DirsToVisit;
    DirsToVisit.AddItem(CookedPath);
    DirsToVisit.AddItem(appGameDir() + TEXT("CookedPCConsoleDLC\\"));

    while (DirsToVisit.Num())
    {
        FString Dir = DirsToVisit.Pop();
        TArray<FString> Found;
        GFileManager->FindFiles(Found, *(Dir + TEXT("*.upk")), TRUE, FALSE);
        for (INT i = 0; i < Found.Num(); ++i)
            OutPackages.AddItem(Dir + Found(i));
        TArray<FString> SubDirs;
        GFileManager->FindFiles(SubDirs, *(Dir + TEXT("*")), FALSE, TRUE);
        for (INT i = 0; i < SubDirs.Num(); ++i)
            DirsToVisit.AddItem(Dir + SubDirs(i) + TEXT("\\"));
    }
}

// Packages scanned so far; written by the scan thread, read by the game thread
// for the progress bar. Declared here (ahead of the other scan-state globals
// further below) because FPropsScanThread::Run() writes to it directly.
static volatile INT GScanCursor = 0;

// ---------------------------------------------------------------------------
// Background scan thread — the whole scan (enumerate packages, read/parse every
// .upk) runs off the game thread. None of it touches UObject/FName/GWorld: it's
// plain file I/O (FArchive/GFileManager) plus manual byte parsing into local
// TArrays, which is exactly why this path was written by hand instead of going
// through ULinkerLoad in the first place. The game thread only polls bDone and,
// once true, copies Results out under the lock.
// ---------------------------------------------------------------------------
class FPropsScanThread : public FRunnable
{
public:
    FPropsScanThread() : bStopRequested(FALSE), bDone(FALSE), ScanTotal(0) {}

    virtual UBOOL Init() { return TRUE; }

    virtual DWORD Run()
    {
        TArray<FString> Packages;
        PropsEnumeratePackages(Packages);

        {
            FScopeLock SL(&Lock);
            ScanTotal = Packages.Num();
        }

        for (INT i = 0; i < Packages.Num() && !bStopRequested; ++i)
        {
            const FString& FullPath = Packages(i);
            char PkgBuf[128] = {};
            FString Tmp = FPackageFileCache::PackageFromPath(*FullPath);
            appStrncpyANSI(PkgBuf, TCHAR_TO_ANSI(*Tmp), ARRAY_COUNT(PkgBuf));
            PropsScanPackage(FullPath, PkgBuf, Results);
            GScanCursor = i + 1;
        }

        if (!bStopRequested) bDone = TRUE;
        return 0;
    }

    virtual void Stop() { bStopRequested = TRUE; }
    virtual void Exit() {}

    volatile UBOOL bStopRequested;
    volatile UBOOL bDone;
    INT            ScanTotal;         // guarded by Lock
    FCriticalSection   Lock;          // guards ScanTotal/Results while the thread is running
    TArray<FPropEntry> Results;
};

// ---------------------------------------------------------------------------
// Scan state (globals) + LRU mesh cache
// ---------------------------------------------------------------------------

static TArray<FPropEntry> GPropList;
static TMap<DWORD,BYTE>   GSeenMeshName;
static bool               GScanDone   = false;

static FPropsScanThread*  GPropsScanThread         = NULL;
static FRunnableThread*   GPropsScanRunnableThread = NULL;

static FPropsLRUEntry GPropsLRU[PROPS_LRU_SIZE]; // zero-initialized statically; Ticket==0 means empty slot
static int            GPropsLRUTicket = 0;

static void PropsLRU_Shutdown()
{
    for (int i = 0; i < PROPS_LRU_SIZE; i++)
    {
        if (GPropsLRU[i].Mesh)
            GPropsLRU[i].Mesh->RemoveFromRoot();
        appMemzero(&GPropsLRU[i], sizeof(GPropsLRU[i]));
    }
    GPropsLRUTicket = 0;
}

// bFound=true means slot exists (Mesh may be NULL if asset not found on disk).
static UStaticMesh* PropsLRU_Find(const char* Pkg, const char* Mesh, bool& bFound)
{
    bFound = false;
    for (int i = 0; i < PROPS_LRU_SIZE; i++)
    {
        if (GPropsLRU[i].Ticket == 0) continue;
        if (appStrcmpANSI(GPropsLRU[i].PackageName, Pkg)  == 0 &&
            appStrcmpANSI(GPropsLRU[i].MeshName,    Mesh) == 0)
        {
            GPropsLRU[i].Ticket = ++GPropsLRUTicket;
            bFound = true;
            return GPropsLRU[i].Mesh;
        }
    }
    return NULL;
}

static void PropsLRU_Insert(const char* Pkg, const char* Mesh, UStaticMesh* M)
{
    int Slot = 0, OldestTicket = INT_MAX;
    for (int i = 0; i < PROPS_LRU_SIZE; i++)
    {
        if (GPropsLRU[i].Ticket == 0) { Slot = i; break; }
        if (GPropsLRU[i].Ticket < OldestTicket) { OldestTicket = GPropsLRU[i].Ticket; Slot = i; }
    }
    if (GPropsLRU[Slot].Mesh) GPropsLRU[Slot].Mesh->RemoveFromRoot();
    appStrncpyANSI(GPropsLRU[Slot].PackageName, Pkg,  ARRAY_COUNT(GPropsLRU[Slot].PackageName));
    appStrncpyANSI(GPropsLRU[Slot].MeshName,    Mesh, ARRAY_COUNT(GPropsLRU[Slot].MeshName));
    GPropsLRU[Slot].Mesh   = M;
    GPropsLRU[Slot].Ticket = ++GPropsLRUTicket;
    if (M) M->AddToRoot();
}

static void PropsStopScanThread()
{
    if (GPropsScanRunnableThread)
    {
        GPropsScanThread->Stop();
        GPropsScanRunnableThread->WaitForCompletion();
        GThreadFactory->Destroy(GPropsScanRunnableThread);
        GPropsScanRunnableThread = NULL;
    }
    delete GPropsScanThread;
    GPropsScanThread = NULL;
}

static void PropsBuildPackageList()
{
    GPropList.Empty();
    GSeenMeshName.Empty();
    GScanCursor = 0;

    PropsStopScanThread(); // safety: shouldn't normally be one running already

    GPropsScanThread = new FPropsScanThread();
    GPropsScanRunnableThread = GThreadFactory->CreateThread(
        GPropsScanThread, TEXT("OLPropsScanThread"), FALSE, FALSE, 0, TPri_BelowNormal);
}

// ---------------------------------------------------------------------------
// Asset loading helpers
// ---------------------------------------------------------------------------

static UStaticMesh* PropsLoadMesh(const FPropEntry& E)
{
    bool bFound = false;
    UStaticMesh* Cached = PropsLRU_Find(E.MeshPackage, E.MeshName, bFound);
    if (bFound) return Cached;
    UStaticMesh* M = Cast<UStaticMesh>(Utils::LoadObjectFromModPackage(
        FString(ANSI_TO_TCHAR(E.MeshPackage)), FString(ANSI_TO_TCHAR(E.MeshName)),
        UStaticMesh::StaticClass()));
    PropsLRU_Insert(E.MeshPackage, E.MeshName, M);
    return M;
}

// ---------------------------------------------------------------------------
// UI state
// ---------------------------------------------------------------------------

// View: list of placed props, or picker for spawning a new one.
enum EPropsView { PV_List, PV_New };
static EPropsView         GPropsView              = PV_List;

static char               GPropsSearch[128]       = {};
static int                GPropsSelectedIdx        = -1;  // index into GPropList (picker)
static int                GPropsPreviewPendingIdx  = -1;
static int                GPropsPreviewLoadedIdx   = -1;
static bool               GPropsPreviewLoading     = false; // async load in flight
static bool               GPropsHasCollision       = true;
static FOLPreviewInstance GPropsPreview;

static FPlacedPropEntry GPlacedProps[MAX_PLACED_PROPS];
static int              GPlacedCount = 0;

// ---------------------------------------------------------------------------
// Placed props — snapshot + delete
// ---------------------------------------------------------------------------

static char GPendingDeleteProp[64] = {};
static void Action_DeletePlacedProp()
{
    if (!GWorld || GPendingDeleteProp[0] == '\0') return;
    FName Target(ANSI_TO_TCHAR(GPendingDeleteProp));
    GPendingDeleteProp[0] = '\0';
    for (FActorIterator It; It; ++It)
    {
        AActor* A = *It;
        if (!A || A->IsPendingKill()) continue;
        if (A->Tag != FName(TEXT("CustomSpawnedProp"))) continue;
        if (A->GetFName() != Target) continue;
        GWorld->DestroyActor(A);
        return;
    }
}

static void RebuildPlacedPropList()
{
    GPlacedCount = 0;
    if (!GWorld) return;
    for (FActorIterator It; It; ++It)
    {
        AActor* A = *It;
        if (!A || A->IsPendingKill()) continue;
        if (A->Tag != FName(TEXT("CustomSpawnedProp"))) continue;
        if (GPlacedCount >= MAX_PLACED_PROPS) break;
        FPlacedPropEntry& E = GPlacedProps[GPlacedCount++];
        appStrncpyANSI(E.Name, TCHAR_TO_ANSI(*A->GetName()), ARRAY_COUNT(E.Name));
        E.ActorFName   = A->GetFName();
        E.bHasCollision = A->bCollideActors;
        E.MeshName[0]   = '\0';
        E.PackageName[0] = '\0';
        ADynamicSMActor* SMA = Cast<ADynamicSMActor>(A);
        if (SMA && SMA->StaticMeshComponent && SMA->StaticMeshComponent->StaticMesh)
        {
            UStaticMesh* SM = SMA->StaticMeshComponent->StaticMesh;
            appStrncpyANSI(E.MeshName,    TCHAR_TO_ANSI(*SM->GetName()),                ARRAY_COUNT(E.MeshName));
            appStrncpyANSI(E.PackageName, TCHAR_TO_ANSI(*SM->GetOutermost()->GetName()), ARRAY_COUNT(E.PackageName));
        }
    }
}

// ---------------------------------------------------------------------------
// Placement
// ---------------------------------------------------------------------------

static int GPropPlacePendingIdx = -1;

static void Action_BeginPropPlacement(const FPropEntry& E)
{
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC || !GWorld) return;

    UClass* SMActorClass = FindObject<UClass>(ANY_PACKAGE, TEXT("DynamicSMActor_Spawnable"));
    if (!SMActorClass) return;

    FVector CamLoc; FRotator CamRot;
    PC->eventGetPlayerViewPoint(CamLoc, CamRot);
    FVector SpawnLoc = CamLoc + FVector(0, 0, -30.f);

    AActor* Ghost = GWorld->SpawnActor(SMActorClass, NAME_None, SpawnLoc, FRotator(0,0,0),
        NULL, TRUE, FALSE, NULL, NULL, TRUE);
    if (!Ghost) return;

    Ghost->bNoDelete = FALSE;
    Ghost->setPhysics(PHYS_None);

    ADynamicSMActor* SMA = Cast<ADynamicSMActor>(Ghost);
    if (SMA && SMA->StaticMeshComponent)
    {
        // Disable component collision flags first, then call SetCollision so
        // ConditionalUpdateComponents re-attaches the primitive with no collision.
        SMA->StaticMeshComponent->CollideActors      = FALSE;
        SMA->StaticMeshComponent->BlockActors        = FALSE;
        SMA->StaticMeshComponent->BlockZeroExtent    = FALSE;
        SMA->StaticMeshComponent->BlockNonZeroExtent = FALSE;
        SMA->StaticMeshComponent->BlockRigidBody     = FALSE;

        UStaticMesh* M = PropsLoadMesh(E);
        if (M)
            SMA->StaticMeshComponent->SetStaticMesh(M);
    }
    Ghost->SetCollision(FALSE, FALSE, Ghost->bIgnoreEncroachers);

    PC->eventBeginPlacementMode(Ghost);
}

static void Action_BeginPropPlacementEnqueued()
{
    const int Idx = GPropPlacePendingIdx;
    GPropPlacePendingIdx = -1;
    if (Idx < 0 || Idx >= GPropList.Num()) return;
    Action_BeginPropPlacement(GPropList(Idx));
}

static bool ConfirmPropPlacement(AOLPlayerController* PC)
{
    ADynamicSMActor* PlacedSMA = Cast<ADynamicSMActor>(PC->PlacementGhost);
    if (!PlacedSMA) return false;

    PC->bPlacementConfirmPending = FALSE;
    PC->PlacementGhost->Tag = FName(TEXT("CustomSpawnedProp"));

    AActor* PlacedActor = PC->PlacementGhost;
    const int SelIdx = GPropsSelectedIdx;
    PC->eventDetachPlacementMode();

    // Apply final physics/collision.
    // Component flags must be set BEFORE SetCollision so ConditionalUpdateComponents
    // sees the correct CollideActors value when re-attaching the primitive.
    //
    // DynamicSMActor::Detach re-enables PHYS_RigidBody when a pawn steps off,
    // which sends the prop flying. Disabling bPawnCanBaseOn prevents pawns from
    // ever Attach()-ing to this actor, breaking that cycle entirely.
    // bWorldGeometry=TRUE makes SetBase skip eventAttach/eventDetach on this actor,
    // so DynamicSMActor::Detach never fires SetPhysics(PHYS_RigidBody).
    // bPawnCanBaseOn=TRUE so BaseChange does not JumpOffPawn (which felt like landing on an enemy).
    PlacedActor->bWorldGeometry   = TRUE;
    PlacedActor->bNoEncroachCheck = TRUE;
    PlacedActor->setPhysics(PHYS_None);
    const UBOOL bCol = GPropsHasCollision ? TRUE : FALSE;
    if (PlacedSMA->StaticMeshComponent)
    {
        UStaticMeshComponent* SMC = PlacedSMA->StaticMeshComponent;
        SMC->CollideActors      = bCol;
        SMC->BlockActors        = bCol;
        SMC->BlockZeroExtent    = bCol;
        SMC->BlockNonZeroExtent = bCol;
        SMC->SetBlockRigidBody(bCol);
    }
    PlacedActor->SetCollision(bCol, bCol, PlacedActor->bIgnoreEncroachers);
    if (SelIdx >= 0 && SelIdx < GPropList.Num())
    {
        GPropPlacePendingIdx = SelIdx;
        OLImGui_EnqueueCall(Action_BeginPropPlacementEnqueued);
    }
    return true;
}

// ---------------------------------------------------------------------------
// Preview loading — staged (non-blocking) mesh load for the hover preview.
// ---------------------------------------------------------------------------

struct FPropPreviewLoad
{
    int  EntryIdx;
    bool bDone;
    bool bStarted;   // staged loader has been kicked off
    FPropPreviewLoad() : EntryIdx(-1), bDone(false), bStarted(false) {}
};
static FPropPreviewLoad GPropLoad;

// Push a finished mesh into the preview scene and mark the load complete.
static void ApplyPropPreviewMesh(UStaticMesh* M)
{
    if (!GPropsPreview.IsInited()) GPropsPreview.Init();
    GPropsPreview.SetStaticMesh(M);

    if (M && M->LODModels.Num() > 0)
    {
        const TArray<FStaticMeshElement>& Elems = M->LODModels(0).Elements;
        for (INT mi = 0; mi < Elems.Num(); ++mi)
            if (Elems(mi).Material)
                GPropsPreview.SetStaticMeshMaterial(Elems(mi).MaterialIndex, Elems(mi).Material);
    }

    GPropsPreviewLoadedIdx = GPropLoad.EntryIdx;
    GPropsPreviewLoading   = false;
    GPropLoad.bDone        = true;
}

// Advance the staged mesh load by one step.  Returns true when the preview is
// ready (or failed); false means IO is still in flight and we retry next tick.
// The game thread never blocks on disk here.
static bool StepPropPreviewLoad()
{
    if (GPropLoad.bDone || GPropLoad.EntryIdx < 0) return true;

    const FPropEntry& E = GPropList(GPropLoad.EntryIdx);

    // Cache hit: mesh (with materials already patched) is still in the LRU.
    if (!GPropLoad.bStarted)
    {
        bool bFound = false;
        UStaticMesh* Cached = PropsLRU_Find(E.MeshPackage, E.MeshName, bFound);
        if (bFound)
        {
            ApplyPropPreviewMesh(Cached);
            return true;
        }
        Utils::BeginStaticMeshLoad(FString(ANSI_TO_TCHAR(E.MeshPackage)),
                                   FString(ANSI_TO_TCHAR(E.MeshName)));
        GPropLoad.bStarted = true;
        return false;
    }

    UStaticMesh* M = NULL;
    if (!Utils::StepStaticMeshLoad(M)) return false;   // still loading

    // Only cache real results: caching NULL would pin "No Preview" on this entry
    // for the rest of the session, including loads abandoned by fast scrolling.
    if (M) PropsLRU_Insert(E.MeshPackage, E.MeshName, M);
    ApplyPropPreviewMesh(M);
    return true;
}

static void StartPropPreviewLoad(int Idx)
{
    if (Idx < 0 || Idx >= GPropList.Num()) return;
    // Abandon any in-flight load and release its precache buffers.
    if (GPropsPreviewLoading) Utils::ResetStaticMeshLoad();
    GPropLoad            = FPropPreviewLoad();
    GPropLoad.EntryIdx   = Idx;
    GPropsPreviewLoading = true;
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

// Scan tick: handler registration, scan-thread completion polling, placed-prop
// list rebuild. Must run every game tick regardless of menu state.
void OLSpawns_ScanProps()
{
    static bool bHandlerRegistered = false;
    if (!bHandlerRegistered)
    {
        OLSpawns_RegisterConfirmHandler(ConfirmPropPlacement);
        bHandlerRegistered = true;
    }

    if (!GScanDone && GPropsScanThread && GPropsScanThread->bDone)
    {
        // Thread finished — pull its results out under the lock, dedup by mesh
        // name (cheap: this runs once, not per-export), sort, and publish.
        TArray<FPropEntry> Raw;
        {
            FScopeLock SL(&GPropsScanThread->Lock);
            Raw = GPropsScanThread->Results;
        }
        PropsStopScanThread();

        GSeenMeshName.Empty();
        for (INT i = 0; i < Raw.Num(); ++i)
        {
            DWORD Hash = HashMeshNameAnsi(Raw(i).MeshName);
            if (GSeenMeshName.Find(Hash)) continue;
            GSeenMeshName.Set(Hash, 1);
            GPropList.AddItem(Raw(i));
        }

        struct FSortCmp { static INT CDECL Cmp(const void* A, const void* B) {
            return appStrcmpANSI(((const FPropEntry*)A)->MeshName, ((const FPropEntry*)B)->MeshName);
        }};
        appQsort(GPropList.GetTypedData(), GPropList.Num(), sizeof(FPropEntry), FSortCmp::Cmp);
        GScanDone = true;
    }

    // Rebuild placed-prop list every 30 ticks — FActorIterator is expensive.
    static INT GPlacedRebuildTick = 0;
    if (++GPlacedRebuildTick >= 30) { GPlacedRebuildTick = 0; RebuildPlacedPropList(); }
}

// Preview tick: mesh load and render — call only while ImGui menu is open.
void OLSpawns_TickProps()
{
    // Only ever one load in flight: a new hover cancels the previous one outright,
    // so scrolling past entries costs nothing and the entry under the cursor wins.
    if (GPropsPreviewPendingIdx >= 0 && GPropsPreviewPendingIdx < GPropList.Num()
        && GPropsPreviewPendingIdx != GPropsPreviewLoadedIdx
        && GPropsPreviewPendingIdx != GPropLoad.EntryIdx)
    {
        StartPropPreviewLoad(GPropsPreviewPendingIdx);
        GPropsPreviewPendingIdx = -1;
    }

    // Advance the active load by one step per tick.
    if (GPropsPreviewLoading)
        StepPropPreviewLoad();

    if (GPropsPreview.IsInited() && GPropsPreviewLoadedIdx >= 0)
        GPropsPreview.Tick(GOLPreviewYaw, 50.f);
}

// ---------------------------------------------------------------------------
// Public: load a random prop StaticMesh from GPropList (hub tile preview).
// Restricted to the base OLGame package so the tile doesn't flash meshes from
// DLC/mod art packages that may not always be loaded/available.
// Returns NULL if no matching entry exists or the mesh fails to load.
// ---------------------------------------------------------------------------

UStaticMesh* OLSpawns_Props_GetRandomMesh()
{
    if (GPropList.Num() == 0)
        return NULL;

    TArray<INT> Candidates;
    for (INT i = 0; i < GPropList.Num(); ++i)
        if (appStricmp(GPropList(i).PackageName, "OLGame") == 0)
            Candidates.AddItem(i);

    if (Candidates.Num() == 0)
        return NULL;

    int Idx = Candidates(appRand() % Candidates.Num());
    return PropsLoadMesh(GPropList(Idx));
}

// ---------------------------------------------------------------------------
// Public: kick off scan on menu open (called from OLImGui_ToggleOverlay).
// Safe to call multiple times — no-ops once scan has started.
// ---------------------------------------------------------------------------

void OLSpawns_Props_BeginScan()
{
    if (GScanDone || GPropsScanThread)
        return;
    PropsBuildPackageList();
}

// ---------------------------------------------------------------------------
// Shutdown
// ---------------------------------------------------------------------------

void OLSpawns_ShutdownProps()
{
    PropsStopScanThread();
    GPropsPreview.Shutdown();
    PropsLRU_Shutdown();
    // Release the loader precache buffers now that nothing is in flight.
    Utils::FlushStaticMeshLoadCache();
    GPropList.Empty();
    GSeenMeshName.Empty();
    GScanCursor             = 0;
    GScanDone               = false;
    GPropsView              = PV_List;
    GPropsSelectedIdx       = -1;
    GPropsPreviewPendingIdx = -1;
    GPropsPreviewLoadedIdx  = -1;
    GPropsPreviewLoading    = false;
    GPropLoad               = FPropPreviewLoad();
    GPlacedCount            = 0;
    GPendingDeleteProp[0]   = '\0';
}

// ---------------------------------------------------------------------------
// Draw — List view (placed props)
// ---------------------------------------------------------------------------

static FOLRowMultiSelect GPlacedPropSel;

static void DrawPropList()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        OLSpawns_BackToHub();
        return;
    }

    const float PlusH = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("##placedprops", ImVec2(0, -PlusH), false);

    if (ImGui::BeginTable("##ptbl", 4,
        ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn(OLLocale_T("spawns.props.col.mesh"),      ImGuiTableColumnFlags_WidthStretch, 3.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.props.col.package"),   ImGuiTableColumnFlags_WidthStretch, 2.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.props.col.collision"), ImGuiTableColumnFlags_WidthFixed,  60.f);
        ImGui::TableSetupColumn(OLLocale_T("spawns.props.col.actions"),   ImGuiTableColumnFlags_WidthFixed,  50.f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        OL_ROW_MULTISELECT_BEGIN(GPlacedPropSel, GPlacedCount);

        for (int i = 0; i < GPlacedCount; i++)
        {
            const FPlacedPropEntry& PE = GPlacedProps[i];
            ImGui::TableNextRow();
            ImGui::PushID(i);

            ImGui::TableSetColumnIndex(0);
            OL_ROW_SELECTABLE(GPlacedPropSel, PE.MeshName[0] ? PE.MeshName : PE.Name, i);

            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("%s", PE.PackageName);

            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(PE.bHasCollision ? OLLocale_T("common.yes") : OLLocale_T("common.no"));

            ImGui::TableSetColumnIndex(3);
            char PopupId[32];
            _snprintf(PopupId, sizeof(PopupId), "##pmenu%d", i);
            if (OL_ROW_MENU_BEGIN(PopupId))
            {
                OL_ROW_MENU_ITEM_DANGER("spawns.action.delete", GPendingDeleteProp, ARRAY_COUNT(GPendingDeleteProp), PE.Name, Action_DeletePlacedProp);
                OL_ROW_MENU_END;
            }

            ImGui::PopID();
        }
        OL_ROW_MULTISELECT_END(GPlacedPropSel);

        if (OL_ROW_MULTIMENU_BEGIN(GPlacedPropSel, "##pbulkmenu"))
        {
            OL_ROW_MULTIMENU_ITEM_DANGER("spawns.action.delete", GPlacedPropSel, _r, GPlacedProps[_r].Name, GPendingDeleteProp, ARRAY_COUNT(GPendingDeleteProp), Action_DeletePlacedProp);
            OL_ROW_MENU_END;
        }
        ImGui::EndTable();
    }

    ImGui::EndChild();

    if (ImGui::Button("+", ImVec2(-1.f, 0.f)))
        GPropsView = PV_New;
}

// ---------------------------------------------------------------------------
// Draw — New view (picker + preview popup)
// ---------------------------------------------------------------------------

static void DrawPropNew()
{
    if (ImGui::Button(OLLocale_T("common.back")))
    {
        GPropsView = PV_List;
        return;
    }

    ImGui::Spacing();

    if (!GScanDone)
    {
        const int Cursor = GScanCursor;
        const int Total  = GPropsScanThread ? GPropsScanThread->ScanTotal : 0;
        const float Frac = Total > 0 ? (float)Cursor / (float)Total : 0.f;
        char Buf[32]; _snprintf(Buf, sizeof(Buf), "%d / %d", Cursor, Total);
        ImGui::ProgressBar(Frac, ImVec2(-1.f, 0.f), Buf);
        ImGui::Spacing();
    }

    OL_SEARCH("##props_search", "spawns.props.search", GPropsSearch, ARRAY_COUNT(GPropsSearch));
    ImGui::Spacing();

    const float BottomH = ImGui::GetFrameHeightWithSpacing() * 2.f + ImGui::GetStyle().ItemSpacing.y * 3.f;
    ImGui::BeginChild("##props_list", ImVec2(0, -BottomH), false);

    const bool bFilter = (GPropsSearch[0] != '\0');
    for (int i = 0; i < GPropList.Num(); i++)
    {
        const FPropEntry& E = GPropList(i);
        if (bFilter)
        {
            FString Haystack(ANSI_TO_TCHAR(E.MeshName));
            FString Needle  (ANSI_TO_TCHAR(GPropsSearch));
            if (Haystack.InStr(Needle, FALSE, TRUE) == INDEX_NONE) continue;
        }

        const bool bSelected = (GPropsSelectedIdx == i);
        if (ImGui::Selectable(E.MeshName, bSelected))
        {
            // Single click = begin ghost placement immediately.
            GPropsSelectedIdx    = i;
            GPropPlacePendingIdx = i;
            OLImGui_EnqueueCall(Action_BeginPropPlacementEnqueued);
        }

        // Hover = load preview and show popup.
        if (ImGui::IsItemHovered())
        {
            GPropsPreviewPendingIdx = i;

            ImGui::BeginTooltip();
            void* Tex = (GPropsPreviewLoadedIdx == i && GPropsPreview.IsInited())
                        ? GPropsPreview.GetImTextureID() : NULL;
            const float PrevSize = 160.f;
            if (Tex)
            {
                ImGui::Image((ImTextureID)Tex, ImVec2(PrevSize, PrevSize));
            }
            else
            {
                ImGui::Dummy(ImVec2(PrevSize, PrevSize));
                ImDrawList* DL = ImGui::GetWindowDrawList();
                ImVec2 P = ImGui::GetItemRectMin();
                ImVec2 Q = ImGui::GetItemRectMax();
                DL->AddRectFilled(P, Q, IM_COL32(25, 25, 30, 255), 4.f);
                const char* Label = GPropsPreviewLoading
                    ? OLLocale_T("common.loading")
                    : OLLocale_T("common.no_preview");
                DL->AddText(ImVec2(P.x + 4.f, P.y + 4.f), IM_COL32(80, 80, 80, 255), Label);
            }
            ImGui::TextDisabled("%s", E.PackageName);
            ImGui::EndTooltip();
        }
    }
    ImGui::EndChild();

    ImGui::Spacing();
    OL_T_L_BOOL("spawns.props.has_collision", &GPropsHasCollision, NULL);
}

// ---------------------------------------------------------------------------
// Tab entry point
// ---------------------------------------------------------------------------

void OLImGui_TabSpawns_Props()
{
    switch (GPropsView)
    {
    case PV_New:  DrawPropNew();  break;
    default:      DrawPropList(); break;
    }
}
