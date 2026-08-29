/*=============================================================================
    OLImGui_Console.cpp -- console window anchored below the main OpenOL window.

    Thread model:
      - Game thread: OLImGui_Console_Tick() copies UConsole::Scrollback into
        GConsoleSnap and flips GConsoleSnapReady.
      - Render thread: OLImGui_Console_Draw() reads GConsoleSnap.
      - Execute: enqueued onto game thread via OLImGui_EnqueueCall.
=============================================================================*/

#include "OLImGui_Tabs.h"
#include "OLImGui_Console.h"

// ---------------------------------------------------------------------------
// Scrollback snapshot (game thread writes, render thread reads)
// ---------------------------------------------------------------------------

#define CONSOLE_SNAP_LINES  64
#define CONSOLE_LINE_LEN    256
#define CONSOLE_AC_MAX      16  // max autocomplete suggestions to show

struct FConsoleSnap
{
    char Lines[CONSOLE_SNAP_LINES][CONSOLE_LINE_LEN];
    int  Count;
    char History[16][CONSOLE_LINE_LEN];
    int  HistoryTop;
    int  HistoryBot;
    char AC[CONSOLE_AC_MAX][CONSOLE_LINE_LEN]; // autocomplete suggestions
    int  ACCount;
    int  ACTotal; // total matches (may exceed CONSOLE_AC_MAX)
};

static FConsoleSnap  GConsoleSnapA;
static FConsoleSnap  GConsoleSnapB;
static FConsoleSnap* GConsoleSnapFront = &GConsoleSnapA; // render thread reads
static FConsoleSnap* GConsoleSnapBack  = &GConsoleSnapB; // game thread writes
static volatile int  GConsoleSnapReady = 0;

// Called from game-thread ticker every frame.
void OLImGui_Console_Tick()
{
    if (!GEngine || !GEngine->GameViewport) return;
    UConsole* Con = GEngine->GameViewport->ViewportConsole;
    if (!Con) return;

    // --- Scrollback ---
    {
        int N = Con->Scrollback.Num();
        int Out = 0;
        int Start = N < CONSOLE_SNAP_LINES ? 0 : N - CONSOLE_SNAP_LINES;
        for (int i = Start; i < N && Out < CONSOLE_SNAP_LINES; ++i)
        {
            const FString& Str = Con->Scrollback(i);
            int Len = Str.Len();
            if (Len <= 0) continue;
            if (Len >= CONSOLE_LINE_LEN) Len = CONSOLE_LINE_LEN - 1;
            for (int j = 0; j < Len; ++j)
                GConsoleSnapBack->Lines[Out][j] = (char)(*Str)[j];
            GConsoleSnapBack->Lines[Out][Len] = '\0';
            ++Out;
        }
        GConsoleSnapBack->Count = Out;
    }

    // --- History ---
    GConsoleSnapBack->HistoryTop = Con->HistoryTop;
    GConsoleSnapBack->HistoryBot = Con->HistoryBot;
    for (int i = 0; i < 16; ++i)
    {
        const FString& H = Con->History[i];
        int Len = H.Len();
        if (Len >= CONSOLE_LINE_LEN) Len = CONSOLE_LINE_LEN - 1;
        for (int j = 0; j < Len; ++j)
            GConsoleSnapBack->History[i][j] = (char)(*H)[j];
        GConsoleSnapBack->History[i][Len] = '\0';
    }

    // --- Autocomplete ---
    {
        int ACTotal = Con->AutoCompleteIndices.Num();
        GConsoleSnapBack->ACTotal = ACTotal;
        int ACOut = 0;
        for (int i = 0; i < ACTotal && ACOut < CONSOLE_AC_MAX; ++i)
        {
            int Idx = Con->AutoCompleteIndices(i);
            if (Idx < 0 || Idx >= Con->AutoCompleteList.Num()) continue;
            const FString& Cmd = Con->AutoCompleteList(Idx).Command;
            int Len = Cmd.Len();
            if (Len >= CONSOLE_LINE_LEN) Len = CONSOLE_LINE_LEN - 1;
            for (int j = 0; j < Len; ++j)
                GConsoleSnapBack->AC[ACOut][j] = (char)(*Cmd)[j];
            GConsoleSnapBack->AC[ACOut][Len] = '\0';
            ++ACOut;
        }
        GConsoleSnapBack->ACCount = ACOut;
    }

    GConsoleSnapReady = 1;
}

// ---------------------------------------------------------------------------
// UpdateCompleteIndices — runs on game thread, triggered when input changes
// ---------------------------------------------------------------------------

static char SPendingTyped[256] = {};

static void UpdateACOnGameThread()
{
    if (!GEngine || !GEngine->GameViewport) return;
    UConsole* Con = GEngine->GameViewport->ViewportConsole;
    if (!Con) return;
    Con->bAutoCompleteLocked = FALSE;
    Con->TypedStr    = FString(ANSI_TO_TCHAR(SPendingTyped));
    Con->TypedStrPos = Con->TypedStr.Len();
    Con->UpdateCompleteIndices();
}

// ---------------------------------------------------------------------------
// Execute — runs on game thread via EnqueueCall
// ---------------------------------------------------------------------------

static char SPendingExec[256] = {};

static void ExecOnGameThread()
{
    if (SPendingExec[0] == '\0') return;
    AOLPlayerController* PC = Utils::GetOLPC();
    if (!PC) return;

    // Push into UConsole history
    if (GEngine && GEngine->GameViewport)
    {
        UConsole* Con = GEngine->GameViewport->ViewportConsole;
        if (Con)
        {
            Con->History[Con->HistoryTop] = FString(ANSI_TO_TCHAR(SPendingExec));
            Con->HistoryTop = (Con->HistoryTop + 1) % 16;
            if (Con->HistoryTop == Con->HistoryBot)
                Con->HistoryBot = (Con->HistoryBot + 1) % 16;
            Con->HistoryCur = Con->HistoryTop;
        }
    }

    PC->ConsoleCommand(FString(ANSI_TO_TCHAR(SPendingExec)));
    SPendingExec[0] = '\0';
}

// ---------------------------------------------------------------------------
// Local history ring (render thread only, written on submit)
// ---------------------------------------------------------------------------

#define LOCAL_HIST 16
static char SLocalHistory[LOCAL_HIST][256] = {};
static int  SHistCount = 0; // total submitted, capped at LOCAL_HIST
static int  SHistNav   = -1; // -1 = not navigating

static void HistoryPush(const char* Cmd)
{
    int Slot = SHistCount % LOCAL_HIST;
    _snprintf(SLocalHistory[Slot], sizeof(SLocalHistory[0]), "%s", Cmd);
    ++SHistCount;
    SHistNav = -1;
}

static int HistoryCallback(ImGuiInputTextCallbackData* Data)
{
    if (GConsoleSnapFront->ACCount > 0 && Data->BufTextLen > 0) return 0;
    if (SHistCount == 0) return 0;

    int Total = SHistCount < LOCAL_HIST ? SHistCount : LOCAL_HIST;

    if (Data->EventKey == ImGuiKey_UpArrow)
    {
        if (SHistNav == -1)       SHistNav = 0;
        else if (SHistNav < Total - 1) ++SHistNav;
    }
    else if (Data->EventKey == ImGuiKey_DownArrow)
    {
        if (SHistNav > 0) --SHistNav;
        else              SHistNav = -1;
    }

    if (SHistNav == -1)
    {
        Data->DeleteChars(0, Data->BufTextLen);
        return 0;
    }

    int Idx = ((SHistCount - 1 - SHistNav) % LOCAL_HIST + LOCAL_HIST) % LOCAL_HIST;
    const char* Entry = SLocalHistory[Idx];
    int Len = 0;
    while (Entry[Len] && Len < Data->BufSize - 1) ++Len;
    for (int i = 0; i < Len; ++i) Data->Buf[i] = Entry[i];
    Data->Buf[Len]   = '\0';
    Data->BufTextLen = Len;
    Data->BufDirty   = true;
    Data->CursorPos  = Len;
    return 0;
}

// ---------------------------------------------------------------------------
// Input state (render thread)
// ---------------------------------------------------------------------------

static char   SInputBuf[256]  = {};
static bool   SReclaimFocus   = false;
static bool   SScrollToBottom = true;
static int    SACSelected     = 0;
static char   SFillBuf[256]   = {}; // set by AC click; written into InputText via CallbackAlways
static bool   SFillPending    = false;

static int InputCallback(ImGuiInputTextCallbackData* Data)
{
    FConsoleSnap* Snap = GConsoleSnapFront;
    bool bACActive = Snap->ACCount > 0 && Data->BufTextLen > 0;

    // Fill from AC click — writes directly into InputText internal buffer
    if (Data->EventFlag & ImGuiInputTextFlags_CallbackAlways)
    {
        if (SFillPending)
        {
            int Len = 0;
            while (SFillBuf[Len] && Len < Data->BufSize - 1) ++Len;
            Data->DeleteChars(0, Data->BufTextLen);
            Data->InsertChars(0, SFillBuf, SFillBuf + Len);
            Data->CursorPos = Len;
            SFillPending = false;
            // SPendingTyped already set to SFillBuf by the click handler; UpdateAC already queued.
        }
        return 0;
    }

    if (Data->EventFlag & ImGuiInputTextFlags_CallbackHistory)
    {
        if (!bACActive)
            return HistoryCallback(Data);
        if (Data->EventKey == ImGuiKey_UpArrow)
        {
            if (SACSelected > 0) --SACSelected;
        }
        else if (Data->EventKey == ImGuiKey_DownArrow)
        {
            if (SACSelected < Snap->ACCount - 1) ++SACSelected;
        }
        return 0;
    }

    if (Data->EventFlag & ImGuiInputTextFlags_CallbackCompletion)
    {
        if (bACActive)
        {
            const char* Fill = Snap->AC[SACSelected];
            Data->DeleteChars(0, Data->BufTextLen);
            Data->InsertChars(0, Fill);
            SACSelected = 0;
            _snprintf(SPendingTyped, sizeof(SPendingTyped), "%s", Fill);
            OLImGui_EnqueueCall(UpdateACOnGameThread);
        }
        return 0;
    }

    if (Data->EventFlag & ImGuiInputTextFlags_CallbackEdit)
    {
        _snprintf(SPendingTyped, sizeof(SPendingTyped), "%.*s",
                  Data->BufTextLen, Data->Buf);
        OLImGui_EnqueueCall(UpdateACOnGameThread);
        SACSelected = 0;
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Draw — render thread
// ---------------------------------------------------------------------------

void OLImGui_Console_Draw()
{
    if (GConsoleSnapReady)
    {
        Swap(GConsoleSnapFront, GConsoleSnapBack);
        GConsoleSnapReady = 0;
        SScrollToBottom   = true;
    }

    ImGui::BeginChild("##ol_console", ImVec2(0.f, 0.f), false,
                      ImGuiWindowFlags_NoScrollbar |
                      ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::SeparatorText(OLLocale_T("console.title"));

    FConsoleSnap* S = GConsoleSnapFront;

    // Build items pointer array for the widget (stack, valid for this frame)
    const char* ACItems[CONSOLE_AC_MAX];
    for (int i = 0; i < S->ACCount; ++i) ACItems[i] = S->AC[i];
    int ACCount = (S->ACCount > 0 && SPendingTyped[0] != '\0') ? S->ACCount : 0;

    // --- Input field with built-in AC popup ---
    if (SReclaimFocus)
    {
        ImGui::SetKeyboardFocusHere();
        SReclaimFocus = false;
    }

    ImGuiInputTextFlags InputFlags =
        ImGuiInputTextFlags_EnterReturnsTrue    |
        ImGuiInputTextFlags_CallbackHistory     |
        ImGuiInputTextFlags_CallbackEdit        |
        ImGuiInputTextFlags_CallbackCompletion  |
        ImGuiInputTextFlags_CallbackAlways;

    ImGui::SetNextItemWidth(-1.f);
    int ClickedItem = -1;
    bool bSubmit = ImGui::InputTextWithACPopup(
        "##consoleinput", SInputBuf, sizeof(SInputBuf),
        InputFlags, InputCallback, NULL,
        ACItems, ACCount, &SACSelected, &ClickedItem);

    // AC item clicked — fill via CallbackAlways next frame, update AC for new text
    if (ClickedItem >= 0 && ClickedItem < S->ACCount)
    {
        _snprintf(SFillBuf,      sizeof(SFillBuf),      "%s", S->AC[ClickedItem]);
        _snprintf(SPendingTyped, sizeof(SPendingTyped), "%s", S->AC[ClickedItem]);
        SFillPending  = true;
        SReclaimFocus = true;
        OLImGui_EnqueueCall(UpdateACOnGameThread);
    }

    if (bSubmit)
    {
        if (SInputBuf[0] != '\0')
        {
            HistoryPush(SInputBuf);
            _snprintf(SPendingExec, sizeof(SPendingExec), "%s", SInputBuf);
            OLImGui_EnqueueCall(ExecOnGameThread);
            SInputBuf[0]     = '\0';
            SACSelected      = 0;
            SPendingTyped[0] = '\0';
            OLImGui_EnqueueCall(UpdateACOnGameThread);
        }
        SReclaimFocus = true;
    }

    ImGui::EndChild();
}
