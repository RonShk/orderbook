# Tests and benchmarks

## Build (Release)

```
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
```

```
cmake --build cmake-build-release
```

## Correctness tests

```
tests/run_tests.sh
```

Each `cases/<name>.txt` is an order file. `cases/<name>.expected` is the hand-computed output of
`orderbook <name>.txt --print-trades --print-book`. The script diffs the two and prints PASS or FAIL.
To add a case, drop in a new `.txt` and the `.expected` file you worked out by hand.

| Case | What it checks |
|---|---|
| `no_cross` | Nothing trades when the bid is below the ask; print order; cents padding (`99.05`) |
| `exact_fill` | Equal quantities fill completely and leave an empty book |
| `partial_fill` | Leftover quantity rests; trades happen at the resting order's price |
| `time_priority_bids` | Equal-priced bids fill oldest first |
| `time_priority_asks` | Same for asks (the mirrored comparator) |
| `sweep_levels` | A buy walks several ask levels, stops at its limit, then rests |

The 20-order and 7-order files from v1 development (`basic_20`, `basic_7`) still need to be added.

## Output format

stdout:

```
TRADE buy=<id> sell=<id> price=<cents> qty=<n>   (only with --print-trades)
orders <n>
trades <n>
checksum <n>
ASKS ... BIDS                                    (only with --print-book)
```

stderr: `seconds <t>`, the time spent in the `add_order` loop. It goes to stderr so stdout is the same on every run.

**Checksum:** starting from 0, each trade folds in `buyID, sellID, price, quantity` in that order using
`checksum = checksum * 1000003 + value` on a `uint64_t`, which wraps on overflow. A faster version must
reproduce both the trade count and the checksum exactly.

## Benchmarks

```
tests/bench.sh
```

That runs each of `data/orders_{1k,10k,100k,1m,10m}.txt` 5 times and prints a Markdown table with the median
time, the trade count and the checksum. It stops if two runs of the same file disagree.

```
tests/bench.sh 7 1k 100k 1m
```

That runs 7 times each, only on the listed sizes. The data files are gitignored. Never regenerate them between
versions.
