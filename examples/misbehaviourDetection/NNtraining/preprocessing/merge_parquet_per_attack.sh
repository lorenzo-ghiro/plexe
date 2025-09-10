#!/bin/bash

# Check at least one argument is given
if [ $# -lt 1 ]; then
    echo "Usage: $0 <top_level_directory> [num_cores]"
    exit 1
fi

TOP_DIR="$1"
NUM_CORES="${2:-$(nproc)}"  # Use second argument or default to number of CPU cores

# Check if directory exists
if [ ! -d "$TOP_DIR" ]; then
    echo "Error: '$TOP_DIR' is not a valid directory."
    exit 2
fi

find "$TOP_DIR" -mindepth 1 -maxdepth 1 -type d | xargs -P "$NUM_CORES" -I{} bash -c '
    echo "Merging files in: {}"
    python3 mergeparquet.py "{}"
'