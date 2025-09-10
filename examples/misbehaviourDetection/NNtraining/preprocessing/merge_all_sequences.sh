#!/bin/bash

# Usage check
if [ $# -ne 1 ]; then
    echo "Usage: $0 '<glob_pattern>'"
    exit 1
fi

GLOB_PATTERN="$1"

echo "Merging files matching pattern: $GLOB_PATTERN"
python3 mergesequences.py "$GLOB_PATTERN"
