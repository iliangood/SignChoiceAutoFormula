#!/usr/bin/env bash

runs=100

sum_elapsed=0
sum_parse=0
sum_formula=0

for ((i=1;i<=runs;i++)); do
    output=$(zig build run -Dmeasure-time -Doptimize=ReleaseFast -- "$@")

    elapsed=$(echo "$output" | grep elapsed | sed 's/.*:\([0-9.]*\)us/\1/')
    parse=$(echo "$output" | grep parse | sed 's/.*:\([0-9.]*\)us/\1/')
    formula=$(echo "$output" | grep formula | sed 's/.*:\([0-9.]*\)us/\1/')

    sum_elapsed=$(awk "BEGIN {print $sum_elapsed + $elapsed}")
    sum_parse=$(awk "BEGIN {print $sum_parse + $parse}")
    sum_formula=$(awk "BEGIN {print $sum_formula + $formula}")
done

avg_elapsed=$(awk "BEGIN {print $sum_elapsed / $runs}")
avg_parse=$(awk "BEGIN {print $sum_parse / $runs}")
avg_formula=$(awk "BEGIN {print $sum_formula / $runs}")

echo "runs: $runs"
echo "avg elapsed: ${avg_elapsed}us"
echo "avg parse:   ${avg_parse}us"
echo "avg formula: ${avg_formula}us"
