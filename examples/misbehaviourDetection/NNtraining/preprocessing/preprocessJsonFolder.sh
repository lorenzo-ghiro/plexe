#!/bin/bash

# Check arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <folder_path> <num_cores>"
    exit 1
fi

FOLDER="$1"
CORES="$2"

# Check Python script exists
if [ ! -f "json2parquet.py" ]; then
    echo "Error: json2parquet.py not found in current directory."
    exit 1
fi

# Find JSON files and process them in parallel
find "$FOLDER" -type f -name "*.json" | \
xargs -P "$CORES" -I {} python3 json2parquet.py "{}"
