#!/bin/sh
# Runs the test/gwentest commands that "gwentest --list" shows as needing
# no arguments, or only optional ones, and as neither interactive nor
# tools, and reports which of them fail.
#
# Usage: run-gwentest.sh GWENTEST LOGDIR
#
# GWENTEST is the path to test/gwentest in a build directory where make
# has already run; it may be the libtool wrapper script. The commands run
# in the order gwentest lists them, all in the scratch directory
# LOGDIR/run, because some read the files that earlier ones write. Each
# runs with stdin from /dev/null and a time limit, and writes its output
# to LOGDIR/<command>.log. The script exits with status 1 if any command
# fails.
#
# Environment:
#   GWENTEST_TIMEOUT  time limit per command in seconds (default 300)
#   GWENTEST_SKIP     space-separated commands not to run
set -u

if [ $# -ne 2 ]; then
  echo "Usage: $0 GWENTEST LOGDIR" >&2
  exit 2
fi

gwentest=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
logdir=$(mkdir -p "$2" && cd "$2" && pwd)
timeout_secs=${GWENTEST_TIMEOUT:-300}

list=$("$gwentest" --list) || {
  echo "$gwentest --list failed" >&2
  exit 2
}
# Fields: name, arguments (none, required, optional), kind. tr drops the
# carriage returns that gwentest.exe prints on Windows.
commands=$(printf '%s\n' "$list" | tr -d '\r' |
  awk -F '\t' '$2 != "required" && $3 != "interactive" && $3 != "tool" {
    print $1
  }')

pass=0
fail=0
skip=0
failed=

dir="$logdir/run"
rm -rf "$dir"
mkdir -p "$dir"

run_one() {
  name=$1
  # Perl's alarm gives a time limit on both Linux and macOS, which has no
  # timeout(1) by default.
  (cd "$dir" &&
    perl -e 'alarm shift; exec @ARGV or die "exec: $!\n"' \
      "$timeout_secs" "$gwentest" "$name") \
    </dev/null >"$logdir/$name.log" 2>&1
}

while read -r name; do
  [ -n "$name" ] || continue
  case " ${GWENTEST_SKIP:-} " in
    *" $name "*)
      skip=$((skip + 1))
      echo "SKIP: $name"
      continue
      ;;
  esac
  start=$(date +%s)
  run_one "$name"
  rv=$?
  secs=$(($(date +%s) - start))
  if [ "$rv" -eq 0 ]; then
    pass=$((pass + 1))
    echo "PASS: $name (${secs}s)"
  else
    fail=$((fail + 1))
    failed="$failed $name"
    if [ "$rv" -eq 142 ]; then
      echo "FAIL: $name (timed out after ${timeout_secs}s)"
    else
      echo "FAIL: $name (exit status $rv, ${secs}s)"
    fi
    if [ -n "${GITHUB_ACTIONS:-}" ]; then
      echo "::group::Output of gwentest $name"
      tail -n 50 "$logdir/$name.log"
      echo "::endgroup::"
    fi
  fi
done <<EOF
$commands
EOF

echo "gwentest: $pass passed, $fail failed, $skip skipped"
[ "$fail" -eq 0 ] || { echo "Failed:$failed"; exit 1; }
