#!/usr/bin/env bash
# Runs every tests/cases/<name>.txt through the order book and diffs the output
# against tests/cases/<name>.expected. Exits non-zero if any case differs.
#
# usage: tests/run_tests.sh [path-to-orderbook-binary]
#        (default: cmake-build-release/orderbook)
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${1:-$ROOT/cmake-build-release/orderbook}"

if [[ ! -x "$BIN" ]]; then
    echo "binary not found: $BIN (build first, see README in tests/)" >&2
    exit 1
fi

pass=0
fail=0
for input in "$ROOT"/tests/cases/*.txt; do
    name="$(basename "$input" .txt)"
    expected="${input%.txt}.expected"
    if [[ ! -f "$expected" ]]; then
        echo "SKIP $name (no .expected file)"
        continue
    fi

    if diff -u "$expected" <("$BIN" "$input" --print-trades --print-book 2>/dev/null) > /tmp/orderbook_diff_$$; then
        echo "PASS $name"
        pass=$((pass + 1))
    else
        echo "FAIL $name"
        sed 's/^/    /' /tmp/orderbook_diff_$$
        fail=$((fail + 1))
    fi
done
rm -f /tmp/orderbook_diff_$$

echo "$pass passed, $fail failed"
[[ $fail -eq 0 ]]
