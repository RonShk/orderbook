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

Each `cases/<name>.txt` is a command file (format below). `cases/<name>.expected` is the hand-computed output of
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
| `cancel_resting` | A canceled order is gone: a later order that would have matched it rests instead |
| `cancel_not_found` | `NotFound` for a filled order, an unknown id, and the wrong side |
| `cancel_partial` | Canceling a partly filled order removes only what's left; the earlier trade stands |
| `modify_reduce_keeps_priority` | Lower quantity at the same price keeps its place in the queue |
| `modify_increase_loses_priority` | Higher quantity at the same price goes behind the others at that price |
| `modify_noop_keeps_priority` | Same price and quantity is `Modified` and keeps its place |
| `modify_price_no_cross` | A new price that doesn't cross rests behind the orders already at that price |
| `modify_price_cross` | A new price that crosses trades at once, across levels, for both a buy and a sell |
| `modify_rejected_not_found` | Quantity or price of 0 or less is `Rejected`; unknown id and wrong side are `NotFound` |
| `modify_then_cancel` | An order keeps its id after being repriced and resized, so it can still be canceled |
| `modify_after_partial_fill` | The new quantity is the remaining quantity, not the total size |

The 20-order and 7-order files from v1 development (`basic_20`, `basic_7`) still need to be added.

## Input format

One command per line, prices in integer cents. Blank lines are skipped. Any other line that doesn't parse
stops the run and reports the line number.

```
Buy <quantity> <price>                       add
Sell <quantity> <price>                      add
Cancel <id> <Buy|Sell>                       cancel a resting order
Modify <id> <Buy|Sell> <quantity> <price>    set a resting order's remaining quantity and price
```

Adds get ids 1, 2, 3 in file order. Cancel and modify lines refer to those ids and don't use one up, so a file
of only adds has the same ids as in v1. The rules for cancel and modify are in `docs/v2.md`.

## Output format

stdout:

```
TRADE buy=<id> sell=<id> price=<cents> qty=<n>        (only with --print-trades)
CANCEL id=<id> <Canceled|NotFound>                    (only with --print-trades)
MODIFY id=<id> <Modified|NotFound|Rejected>           (only with --print-trades, before its trades)
orders <n>                                            (number of adds)
cancels <n> canceled=<n> not_found=<n>                (only if there were cancels)
modifies <n> modified=<n> not_found=<n> rejected=<n>  (only if there were modifies)
trades <n>
checksum <n>
ASKS ... BIDS                                         (only with --print-book)
```

With `--print-trades`, the trade, cancel and modify lines come out in the order they happened. A file of only
adds prints exactly what v1 printed.

stderr: `seconds <t>`, the time spent running the commands (not reading the file). It goes to stderr so stdout
is the same on every run.

**Checksum:** starting from 0, each event folds its values in with `checksum = checksum * 1000003 + value` on a
`uint64_t`, which wraps on overflow, in the order the events happened:

- a trade folds in `buyID, sellID, price, quantity`
- a cancel folds in `1, id, result`, with `Canceled = 0, NotFound = 1`
- a modify folds in `2, id, result`, with `Modified = 0, NotFound = 1, Rejected = 2`, and then its trades

So a file of only adds gives the same checksum as v1. A faster version must reproduce the summary lines and the
checksum exactly.

## Generating data

```
cmake-build-release/gen <count> <output-file> [--cancel <pct>] [--modify <pct>] [--seed <n>]
```

With no `--cancel` or `--modify`, every line is an add, and the output is byte-identical to the v1 generator's
for the same count and seed (default 43). The v1 files come from, for example:

```
cmake-build-release/gen 1000000 data/orders_1m.txt
```

The mixed files use 20% cancels and 10% modifies:

```
cmake-build-release/gen 1000000 data/mixed_1m.txt --cancel 20 --modify 10
```

Cancels and modifies target one of the last 1000 adds, with that add's side. Half of the modifies keep the
price and half move it by up to a dollar either way. 2% send quantity 0 and are rejected. Adds draw from their
own random stream, so the mix changes how many adds there are but not which adds are generated.

## Benchmarks

```
tests/bench.sh
```

That runs each of `data/orders_{1k,10k,100k,1m,10m}.txt` 5 times and prints a Markdown table with the median
time, the trade count and the checksum. Commands/sec counts every line: adds, cancels and modifies. It stops if
two runs of the same file disagree.

```
tests/bench.sh 7 1k 100k 1m
```

That runs 7 times each, only on the listed sizes.

```
PREFIX=mixed tests/bench.sh 5 1k 10k 100k 1m
```

That runs `data/mixed_<size>.txt` instead. The data files are gitignored. Never regenerate them between
versions.
