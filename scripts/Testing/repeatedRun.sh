#!/bin/bash

# This script repeatedly runs a command and saves all its output to a file.
# This is useful if e.g. trying to statistically verify that a non-deterministic error is fixed.

command=$1
count=$2
result_file=$3

echo "running \`$command\` $count times and saving output to \`$result_file\`."

# run many times and save stdout and stderr to a file
for i in $(seq "$count"); do
    echo "starting iteration $i/$count..."
    printf "\n\n------------- iteration $i/$count -------------\n\n" >> "$result_file"
    $command >> "$result_file" 2>&1
done

echo "finished all $count iterations."