#!/usr/bin/env python3
from sys import argv, exit

import pandas as pd

from glob import glob
from generic_parser import *


def main():
    if len(argv) < 6:
        print("generic-parser.py requires at least 6 parameters")
        exit(1)

    resfolder = argv[1]
    prefix = argv[2]
    outfile = argv[3]
    mapfile = argv[4]
    mapfile = argv[4]
    config = argv[5]
    outtype = argv[6]

    supported = ["csv", "parquet"]
    if outtype not in supported:
        exit(f"type is {outtype} but must be one of the following: {supported}")

    print("merging data")
    print("outfile: ", f"{resfolder}{outfile}")
    files = glob(f"{resfolder}{prefix}*{outtype}")
    print("files: ")
    print(files)

    # load map file
    map_data = parse_map(mapfile)
    # check whether required config exists
    if config not in map_data.keys():
        print("required config {} does not exist in {}".format(config, mapfile))
        exit(1)

    dfs = []
    for file in files:
        if outtype == "csv":
            df = pd.read_csv(file)
        elif outtype == "parquet":
            df = pd.read_parquet(file)

        # get simulation parameters
        vecfile = file.replace(prefix, config).replace(f".{outtype}", ".vec")
        params = get_params(vecfile, map_data[config]["fields"])

        for col in params.columns:
            df[col] = params[col].iloc[0]

        dfs.append(df)

    # Concatenate current df with the whole data
    data = pd.concat(dfs)

    # Flush data to proper outfile
    writeto = resfolder+outfile
    if writeto.endswith(".csv"):
        data.to_csv(writeto, index=False)
    elif writeto.endswith("parquet"):
        data.to_parquet(writeto, index=False)


if __name__ == "__main__":
    main()
