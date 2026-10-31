#!/bin/bash
# Release flow for OpenOL. The version lives in OutlastLauncher/Version.h.
#
#   release.sh bump patch|minor|major [--rc]  raise the version and commit
#   release.sh bump rc|final|X.Y.Z[-rcN]      next release candidate, or the final version of the current one
#   release.sh --dry-run [--skip-build]       run all checks, build and verify; change nothing remotely
#   release.sh draft                          build, verify, push the tag, upload a draft release
#   release.sh publish                        make the draft public
#   release.sh build-archives                 build and package only (for CI, needs no git)
#   release.sh upload-draft <dir>             verify the archives in <dir> and upload a draft (for CI)
#
# The usual order: bump, get it onto main and push, dry-run, draft, try the draft, publish.
# A release candidate (X.Y.Z-rcN) can be drafted from any pushed branch. It is published as a
# prerelease, so only people who installed a candidate by hand are offered newer ones.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
TARGETS="$ROOT/Development/Src/Targets"
VERSION_H="$ROOT/Development/Tools/OutlastLauncher/Version.h"
LAUNCHER_DIR="$ROOT/Development/Tools/OutlastLauncher"
TESTS="$LAUNCHER_DIR/Tests/test_updater.py"
RELAY="Development/Src/Multiplayer/server"
PUBLISH_DIR="$ROOT/PublishTemp"
REPO="ShyKiss/OpenOL"
ASSETS=(OpenOL.zip OpenOL.7z)

cd "$ROOT"

PROBLEMS=0
DRY_RUN=0

die()  { echo "ERROR: $*" >&2; exit 1; }
warn() { echo "WARNING: $*" >&2; }
step() { echo ""; echo "==== $*"; }

# A dry run reports every problem and goes on. A real release stops after preflight.
problem() {
    PROBLEMS=$((PROBLEMS + 1))
    if (( DRY_RUN )); then echo "WOULD BLOCK A RELEASE: $*" >&2; else echo "ERROR: $*" >&2; fi
}

VERSION_RE='^([0-9]+)\.([0-9]+)\.([0-9]+)(-rc([0-9]+))?$'

read_version() {
    sed -n 's/^#define OPENOL_VERSION_STR "\(.*\)"/\1/p' "$VERSION_H"
}

write_version() {  # $1 = X.Y.Z
    [[ "$1" =~ $VERSION_RE ]] || die "Not a version: $1"
    local num="${BASH_REMATCH[1]},${BASH_REMATCH[2]},${BASH_REMATCH[3]},${BASH_REMATCH[5]:-0}"
    sed -i -e "s/^#define OPENOL_VERSION_NUM .*/#define OPENOL_VERSION_NUM $num/" \
           -e "s/^#define OPENOL_VERSION_STR .*/#define OPENOL_VERSION_STR \"$1\"/" \
           -e "s/^#define CURRENT_VERSION .*/#define CURRENT_VERSION L\"$1\"/" "$VERSION_H"
}

is_rc() { [[ "$1" == *-rc* ]]; }

# Sortable form of a version. A final version sorts above its own release candidates,
# which is why plain "sort -V" cannot be used.
version_key() {
    [[ "$1" =~ $VERSION_RE ]] || die "Not a version: $1"
    printf '%06d%06d%06d%06d' "$((10#${BASH_REMATCH[1]}))" "$((10#${BASH_REMATCH[2]}))" \
        "$((10#${BASH_REMATCH[3]}))" "$((10#${BASH_REMATCH[5]:-999999}))"
}

# Succeeds if $1 is strictly higher than $2.
version_gt() {
    [[ "$(version_key "$1")" > "$(version_key "$2")" ]]
}

latest_tag() {
    local tag best=""
    for tag in $(git tag); do
        [[ "${tag#v}" =~ $VERSION_RE ]] || continue
        if [[ -z "$best" ]] || version_gt "${tag#v}" "${best#v}"; then best="$tag"; fi
    done
    echo "$best"
}

cmd_bump() {  # $1 = patch|minor|major|rc|final|X.Y.Z[-rcN], $2 = --rc
    local current next major minor patch rc
    current="$(read_version)"
    [[ "$current" =~ $VERSION_RE ]] || die "Bad version in Version.h: $current"
    major="${BASH_REMATCH[1]}"; minor="${BASH_REMATCH[2]}"; patch="${BASH_REMATCH[3]}"; rc="${BASH_REMATCH[5]}"

    case "${1:-}" in
        patch) next="$major.$minor.$((patch + 1))" ;;
        minor) next="$major.$((minor + 1)).0" ;;
        major) next="$((major + 1)).0.0" ;;
        rc)
            [[ -n "$rc" ]] || die "$current is not a release candidate. Start one with: bump patch|minor|major --rc"
            next="$major.$minor.$patch-rc$((rc + 1))" ;;
        final)
            [[ -n "$rc" ]] || die "$current is already final."
            next="$major.$minor.$patch" ;;
        [0-9]*)
            [[ "$1" =~ $VERSION_RE ]] || die "Not a version: $1"
            next="$1" ;;
        *) die "Usage: release.sh bump patch|minor|major [--rc] | rc | final | X.Y.Z[-rcN]" ;;
    esac
    if [[ "${2:-}" == "--rc" ]]; then
        case "$1" in
            patch|minor|major) next="$next-rc1" ;;
            *) die "--rc only goes with patch, minor or major." ;;
        esac
    fi

    version_gt "$next" "$current" || die "$next is not higher than the current $current"
    local on_main=()
    if [[ "$(git rev-parse --abbrev-ref HEAD)" == "main" ]]; then on_main=(--main); fi
    [[ -z "$(git status --porcelain)" ]] || die "Commit your changes first: git_commit.sh would sweep them into the version commit."

    write_version "$next"
    if ! "$TARGETS/git_commit.sh" "${on_main[@]}" -m "Version $next"; then
        write_version "$current"
        die "Could not commit the version, it stays $current."
    fi
    echo "Version: $current -> $next"
}

preflight() {  # $1 = version
    local version="$1" tag_commit newest
    step "Preflight for $version"

    local tool
    for tool in git gh 7z python3 wine i686-w64-mingw32-g++; do
        command -v "$tool" > /dev/null || problem "$tool is not installed"
    done
    gh auth status > /dev/null 2>&1 || problem "gh is not logged in"

    git fetch -q --tags origin || problem "Cannot fetch from origin"
    git -C "$RELAY" fetch -q --tags origin || problem "Cannot fetch the relay from origin"

    # A final release comes from main. A release candidate may come from any branch.
    local branch
    branch="$(git rev-parse --abbrev-ref HEAD)"
    is_rc "$version" || [[ "$branch" == "main" ]] || problem "Not on main: $branch"
    [[ -z "$(git status --porcelain)" ]] || problem "Working tree has uncommitted changes"
    [[ "$(git rev-parse HEAD)" == "$(git rev-parse -q --verify "origin/$branch" || true)" ]] || problem "HEAD is not origin/$branch: the tag must point at a pushed commit"

    tag_commit="$(git rev-parse -q --verify "refs/tags/$version^{commit}" || true)"
    newest="$(latest_tag)"
    if [[ -n "$tag_commit" ]]; then
        # The same commit is fine: it means an earlier attempt failed after tagging.
        [[ "$tag_commit" == "$(git rev-parse HEAD)" ]] || problem "Tag $version already exists on another commit. Run: release.sh bump"
    elif [[ -n "$newest" ]] && ! version_gt "$version" "${newest#v}"; then
        problem "Version $version is not higher than the newest tag $newest. Run: release.sh bump"
    fi

    if [[ -z "$(git -C "$RELAY" branch -r --contains HEAD 2>/dev/null)" ]]; then
        problem "The relay commit is not on GitHub: clones of the release could not fetch the submodule"
    fi
    # Not fatal: a release may not need a new relay. But hosts on an old one cannot be joined after a protocol change.
    if ! git -C "$RELAY" describe --tags --exact-match HEAD > /dev/null 2>&1; then
        warn "The relay commit has no tag. If its protocol changed, release OpenOL-Relay too."
    fi

    if (( PROBLEMS > 0 && ! DRY_RUN )); then
        die "$PROBLEMS problem(s), nothing was changed."
    fi
}

# Wine servers outlive the build and hold on to stdout, so the output goes to a file, never a pipe.
run_build() {  # $1 = build_wine.sh item, $2 = description
    local log="$BUILD_LOGS/build_$1.log"
    echo "  $2"
    if ! "$TARGETS/build_wine.sh" "$1" > "$log" 2>&1 < /dev/null; then
        tail -20 "$log" >&2
        die "Build failed: $2 (log: $log)"
    fi
}

# The order matters: UnrealScript needs the editor exe and generates the headers Production needs.
build_all() {
    step "Build (logs: $BUILD_LOGS)"
    # MSBuild runs here without file tracking: make sure the new version is compiled in.
    touch "$LAUNCHER_DIR/Updater.cpp" "$LAUNCHER_DIR/OutlastLauncher.rc"

    run_build 11 "openol_relay.lib"
    run_build 12 "imgui.lib"
    run_build 1  "OLGame.exe"
    run_build 7  "UnrealScript"
    if grep -aqE 'Failure - [0-9]+ error' "$BUILD_LOGS/build_7.log"; then
        die "UnrealScript failed to compile (log: $BUILD_LOGS/build_7.log)"
    fi
    run_build 3  "OLGame Production"
    run_build 5  "OutlastLauncher"
}

make_archives() {
    step "Archives"
    "$TARGETS/publish_production.sh" --strict > "$BUILD_LOGS/publish.log" 2>&1 < /dev/null || {
        tail -30 "$BUILD_LOGS/publish.log" >&2
        die "Packaging failed (log: $BUILD_LOGS/publish.log)"
    }
    grep -a 'MISSING' "$BUILD_LOGS/publish.log" || true
}

# Checks $1/OpenOL.zip and .7z with the same code the launcher uses to install them.
verify_archives() {  # $1 = folder, $2 = version
    step "Verify archives in $1"
    7z t "$1/OpenOL.7z" > /dev/null || die "OpenOL.7z is damaged"
    python3 "$TESTS" --archive "$1/OpenOL.zip" --expect-version "$2" || die "Verification failed"
    (cd "$1" && sha256sum "${ASSETS[@]}" && du -h "${ASSETS[@]}")
}

# Downloads the release assets into $2. Works for drafts.
download_assets() {  # $1 = tag, $2 = folder
    local asset
    for asset in "${ASSETS[@]}"; do
        gh release download "$1" -R "$REPO" -p "$asset" -D "$2" --clobber
    done
}

cmd_dry_run() {  # $1 = 1 to skip the build
    local version
    version="$(read_version)"
    DRY_RUN=1
    preflight "$version"
    if (( $1 )); then
        step "Build skipped: checking the binaries already on disk"
    else
        build_all
    fi
    make_archives
    verify_archives "$PUBLISH_DIR" "$version"

    step "Dry run finished for $version: nothing was committed, tagged or uploaded"
    if (( PROBLEMS > 0 )); then
        echo "$PROBLEMS problem(s) would block a real release, see above."
        return 1
    fi
    echo "No problems. Next: release.sh draft"
}

release_state() {  # $1 = version; prints true for a draft, false if public, nothing if absent
    gh release view "$1" -R "$REPO" --json isDraft --jq .isDraft 2>/dev/null || true
}

cmd_draft() {
    local version state
    version="$(read_version)"
    preflight "$version"

    state="$(release_state "$version")"
    [[ "$state" != "false" ]] || die "Release $version is already public. Run: release.sh bump"

    build_all
    make_archives
    verify_archives "$PUBLISH_DIR" "$version"
    upload_draft "$version" "$PUBLISH_DIR"
}

# For CI, where the archives come out of the build container.
cmd_upload_draft() {  # $1 = folder with the archives
    local version
    [[ -d "${1:-}" ]] || die "Usage: release.sh upload-draft <dir>"
    version="$(read_version)"
    [[ "$(release_state "$version")" != "false" ]] || die "Release $version is already public."
    verify_archives "$1" "$version"
    upload_draft "$version" "$1"
}

upload_draft() {  # $1 = version, $2 = folder with the archives
    local version="$1" dir="$2" state tmp asset
    local kind=() files=()
    if is_rc "$version"; then kind=(--prerelease); fi
    for asset in "${ASSETS[@]}"; do files+=("$dir/$asset"); done
    state="$(release_state "$version")"

    step "Tag and draft release $version"
    if ! git rev-parse -q --verify "refs/tags/$version" > /dev/null; then
        git tag -a "$version" -m "OpenOL $version"
    fi
    git push -q origin "refs/tags/$version"

    if [[ "$state" == "true" ]]; then
        gh release upload "$version" -R "$REPO" --clobber "${files[@]}"
    else
        gh release create "$version" -R "$REPO" --draft --verify-tag --generate-notes "${kind[@]}" \
            --title "v$version" "${files[@]}"
    fi

    # What matters is what GitHub stored, not what was sent.
    tmp="$(mktemp -d)"
    download_assets "$version" "$tmp"
    for asset in "${ASSETS[@]}"; do
        cmp "$dir/$asset" "$tmp/$asset" || die "$asset on GitHub differs from the local file"
    done
    rm -f "$tmp/OpenOL.zip" "$tmp/OpenOL.7z"
    rmdir "$tmp"

    step "Draft $version is ready, and invisible to users"
    gh release view "$version" -R "$REPO" --json url --jq .url
    echo "Edit the notes there and try the archive, then: release.sh publish"
}

cmd_publish() {
    local version state tmp latest digest answer
    version="$(read_version)"

    state="$(release_state "$version")"
    [[ "$state" == "true" ]] || die "No draft release for $version. Run: release.sh draft"

    # Check the uploaded files themselves: the local ones may be from another build by now.
    tmp="$(mktemp -d)"
    download_assets "$version" "$tmp"
    verify_archives "$tmp" "$version"

    echo ""
    if is_rc "$version"; then
        echo "This publishes $version as a prerelease. Regular users are not offered it,"
        echo "only those already on a release candidate."
    else
        echo "This offers $version to every user of the launcher. It cannot be taken back from"
        echo "those who have installed it: only a higher version can replace it."
    fi
    printf 'Type %s to publish: ' "$version"
    read -r answer
    if [[ "$answer" != "$version" ]]; then
        rm -f "$tmp/OpenOL.zip" "$tmp/OpenOL.7z"
        rmdir "$tmp"
        die "Not published."
    fi

    if is_rc "$version"; then
        gh release edit "$version" -R "$REPO" --draft=false --prerelease --latest=false > /dev/null
    else
        gh release edit "$version" -R "$REPO" --draft=false --prerelease=false --latest > /dev/null
    fi

    # The launcher requires this exact state: without a digest it refuses the update.
    latest="$(gh api "repos/$REPO/releases/latest" --jq .tag_name)"
    digest="$(gh api "repos/$REPO/releases/tags/$version" --jq '.assets[] | select(.name == "OpenOL.zip") | .digest')"
    local local_digest
    local_digest="sha256:$(sha256sum "$tmp/OpenOL.zip" | cut -d' ' -f1)"
    rm -f "$tmp/OpenOL.zip" "$tmp/OpenOL.7z"
    rmdir "$tmp"

    if is_rc "$version"; then
        [[ "$latest" != "$version" ]] || die "$version was published as the latest release: regular users are being offered it!"
    else
        [[ "$latest" == "$version" ]] || die "Published, but the latest release is $latest, not $version"
    fi
    [[ "$digest" == "$local_digest" ]] || die "Published, but the digest on GitHub is '$digest', expected $local_digest. Launchers will refuse this update."

    step "Published $version"
    gh release view "$version" -R "$REPO" --json url --jq .url
}

BUILD_LOGS="$(mktemp -d)"
case "${1:-}" in
    bump)      shift; cmd_bump "${1:-}" "${2:-}" ;;
    build-archives) build_all; make_archives ;;
    upload-draft)   shift; cmd_upload_draft "${1:-}" ;;
    draft)     cmd_draft ;;
    publish)   cmd_publish ;;
    --dry-run)
        SKIP=0
        [[ "${2:-}" == "--skip-build" ]] && SKIP=1
        cmd_dry_run "$SKIP"
        ;;
    *) sed -n '2,14p' "$0" | sed 's/^# \{0,1\}//'; exit 2 ;;
esac
