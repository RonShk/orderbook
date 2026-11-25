#!/usr/bin/env bash
# Runs the order book RUNS times on each data file and reports the median time.
# Also checks that every run of a file gives the same trade count and checksum.
#
# usage: tests/bench.sh [runs] [sizes...]
#   tests/bench.sh                 # 5 runs each of 1k 10k 100k 1m 10m
#   tests/bench.sh 7 1k 100k 1m    # 7 runs, only those sizes
# env:   BIN=path/to/orderbook  DATA=path/to/data-dir
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${BIN:-$ROOT/cmake-build-release/orderbook}"
DATA="${DATA:-$ROOT/data}"
RUNS="${1:-5}"
shift || true
SIZES=("$@")
[[ ${#SIZES[@]} -eq 0 ]] && SIZES=(1k 10k 100k 1m 10m)

if [[ ! -x "$BIN" ]]; then
    echo "binary not found: $BIN" >&2
    exit 1
fi

echo "machine:  $(uname -srm), $(nproc) cores"
grep -m1 'model name' /proc/cpuinfo 2>/dev/null | sed 's/.*: /cpu:      /'
sysctl -n machdep.cpu.brand_string 2>/dev/null | sed 's/^/cpu:      /'
echo "compiler: $(c++ --version | head -1)"
echo "runs:     $RUNS per size (median reported)"
echo

printf '| %-6s | %10s | %10s | %10s | %12s | %8s | %20s |\n' \
    size orders "median s" "min s" "orders/sec" trades checksum
printf '|%s|%s|%s|%s|%s|%s|%s|\n' \
    -------- ------------ ------------ ------------ -------------- ---------- ----------------------

for size in "${SIZES[@]}"; do
    file="$DATA/orders_$size.txt"
    if [[ ! -f "$file" ]]; then
        echo "missing $file, skipping" >&2
        continue
    fi

    times=()
    ref=""
    for ((r = 1; r <= RUNS; r++)); do
        # stdout has orders/trades/checksum, stderr has "seconds X"
        out="$("$BIN" "$file" 2>"/tmp/orderbook_time_$$")" || { echo "run failed on $file" >&2; exit 1; }
        t="$(awk '/^seconds/ {print $2}' "/tmp/orderbook_time_$$")"
        times+=("$t")
        if [[ -z "$ref" ]]; then
            ref="$out"
        elif [[ "$out" != "$ref" ]]; then
            echo "MISMATCH on $file run $r: output differs from run 1" >&2
            exit 1
        fi
        echo "  $size run $r/$RUNS: ${t}s" >&2
    done

    orders="$(awk '/^orders/ {print $2}' <<< "$ref")"
    trades="$(awk '/^trades/ {print $2}' <<< "$ref")"
    checksum="$(awk '/^checksum/ {print $2}' <<< "$ref")"
    sorted="$(printf '%s\n' "${times[@]}" | sort -g)"
    median="$(awk '{a[NR]=$1} END {if (NR % 2) print a[(NR+1)/2]; else print (a[NR/2] + a[NR/2+1]) / 2}' <<< "$sorted")"
    min="$(head -1 <<< "$sorted")"
    rate="$(awk -v n="$orders" -v t="$median" 'BEGIN {printf "%.0f", n / t}')"

    printf '| %-6s | %10s | %10.4g | %10.4g | %12s | %8s | %20s |\n' \
        "$size" "$orders" "$median" "$min" "$rate" "$trades" "$checksum"
done
rm -f "/tmp/orderbook_time_$$"
