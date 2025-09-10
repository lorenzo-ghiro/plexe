import sys
import glob
import re
import os
import pandas as pd
from pathlib import Path
import code #code.interact(local=dict(globals(), **locals()))

if len(sys.argv) != 2:
    print("Usage: python mergesequences.py '<glob_pattern>'")
    sys.exit(1)

pattern = sys.argv[1]
files = glob.glob(pattern, recursive=True)
if not files:
    print(f"No files matched pattern: {pattern}")
    sys.exit(2)

print(f"Found {len(files)} files. Merging...")

# Detect X or Y sequence type
prefix = None
filename = os.path.basename(files[0]).lower()
if "xsequences" in filename:
    prefix = "xsequences"
elif "ysequences" in filename:
    prefix = "ysequences"

if not prefix:
    print("Could not determine if files are X or Y sequences from filenames.")
    sys.exit(3)

# Extract common suffix pattern from filenames
# Example filenames: xsequences_something_SUFFIX.parquet
suffix = None
match = re.search(r'(x|y)sequences.*?(_[^/\\]+)\.parquet$', filename, re.IGNORECASE)
if match:
    suffix = match.group(2).lstrip('_')

if not suffix:
    print("Could not extract suffixes from filenames.")
    sys.exit(4)


output_file = f"ALL_{prefix}_{suffix}.parquet"
print(f"Output will be saved as: {output_file}")

# Merge all matching files
dfs = []
for file in files:
    try:
        df = pd.read_parquet(file)
        dfs.append(df)
    except Exception as e:
        print(f"Failed to read {file}: {e}")

if not dfs:
    print("No valid Parquet files could be read.")
    sys.exit(5)

merged_df = pd.concat(dfs, ignore_index=True)
Path(output_file).parent.mkdir(parents=True, exist_ok=True)
merged_df.to_parquet(output_file)
print(f"Saved merged file to: {output_file}")
