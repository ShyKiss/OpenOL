#!/bin/bash
# Commit everything that belongs in the repository, relay submodule included.
#
#   git_commit.sh -m "message"          commit on the current branch
#   git_commit.sh -m "message" --push   commit, then push (submodule first)
#   git_commit.sh --push                push only
#   git_commit.sh --dry-run -m "..."    show what would be committed
#
# Never commits to main directly: on main it stops and asks for a branch.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
RELAY="Development/Src/Multiplayer/server"
# A larger commit is more likely the UE3 tree leaking through than real work.
MAX_NEW_FILES=150
MAX_FILE_BYTES=$((50 * 1024 * 1024))

MESSAGE=""
PUSH=0
DRY_RUN=0
while [[ $# -gt 0 ]]; do
    case "$1" in
        -m) MESSAGE="$2"; shift 2 ;;
        --push) PUSH=1; shift ;;
        --dry-run) DRY_RUN=1; shift ;;
        *) echo "Unknown option: $1" >&2; exit 2 ;;
    esac
done
[[ -n "$MESSAGE" || $PUSH -eq 1 ]] || { echo "Nothing to do: pass -m and/or --push" >&2; exit 2; }

cd "$ROOT"

unstage() {  # $1 = repo dir
    git -C "$1" reset -q
    # git_track.py has seen these files; forget that, or the next scan would skip them.
    rm -f "$ROOT/.git/git_track_stamp"
}

check_staged() {  # $1 = repo dir
    local new_count big
    new_count=$(git -C "$1" diff --cached --name-only --diff-filter=A | wc -l)
    if (( new_count > MAX_NEW_FILES )); then
        echo "ABORT: $new_count new files staged in $1 (limit $MAX_NEW_FILES). Check for leaked engine sources." >&2
        unstage "$1"
        exit 1
    fi
    big=$(git -C "$1" diff --cached --name-only --diff-filter=AM -z |
        (cd "$1" && xargs -0 -r stat -c '%s %n' 2>/dev/null) | awk -v m="$MAX_FILE_BYTES" '$1 > m' || true)
    if [[ -n "$big" ]]; then
        echo "ABORT: files over $MAX_FILE_BYTES bytes staged in $1:" >&2
        echo "$big" >&2
        unstage "$1"
        exit 1
    fi
    if git -C "$1" diff --cached -U0 --no-ext-diff --text |
        grep -qE '^\+.*(ghp_[A-Za-z0-9]{20}|github_pat_[A-Za-z0-9_]{20}|BEGIN [A-Z ]*PRIVATE KEY)'; then
        echo "ABORT: staged changes in $1 look like they contain a secret." >&2
        unstage "$1"
        exit 1
    fi
}

commit_repo() {  # $1 = repo dir
    if git -C "$1" diff --cached --quiet; then
        echo "[$1] nothing to commit"
        return
    fi
    if [[ "$(git -C "$1" rev-parse --abbrev-ref HEAD)" == "main" ]]; then
        echo "ABORT: $1 is on main. Create a branch first." >&2
        unstage "$1"
        exit 1
    fi
    if (( DRY_RUN )); then
        echo "[$1] would commit:"
        git -C "$1" diff --cached --stat | tail -25
        unstage "$1"
        return
    fi
    git -C "$1" commit -q -m "$MESSAGE"
    git -C "$1" log --oneline -1 | sed "s|^|[$1] |"
}

if [[ -n "$MESSAGE" ]]; then
    # The relay goes first: the parent commit records the relay commit.
    git -C "$RELAY" add -A
    check_staged "$RELAY"
    commit_repo "$RELAY"

    # Default-deny .gitignore: -A only reaches tracked files and whitelisted own code.
    python3 Development/Src/Targets/git_track.py --add
    git -c advice.addEmbeddedRepo=false add -A
    check_staged .
    commit_repo .
fi

if (( PUSH && ! DRY_RUN )); then
    git -C "$RELAY" push -q -u origin HEAD
    git push -q -u origin HEAD
    echo "Pushed: $(git rev-parse --abbrev-ref HEAD) (relay: $(git -C "$RELAY" rev-parse --abbrev-ref HEAD))"
fi
