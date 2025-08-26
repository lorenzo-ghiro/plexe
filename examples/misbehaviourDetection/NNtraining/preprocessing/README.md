# Preprocessing of VeReMi Data

- How to create compact parquet file representing the whole VeReMi dataset, that is shipped as a collection of Json files
- How VeReMi messages are labeled
- How labelled messages are sorted/aligned/put-in-sequences to be used as input for neural network and rule based MDS


## Convert all Json to parquet

```console
./preprocessJsonFolder.sh <folder> <num_cores>
```

e.g.

```console
./preprocessJsonFolder.sh ../Data 20
```

This bash script detects all JSON files representing log and groundTruth messages of the VeReMi dataset and converts them to a more compact (10x memory saving) format (parquet) to ease next steps of labeling data and the further creation of training sequences.


## Merge parquet

Once the JSON files have been converted to parquet, all `traceJSON` and `traceGroundTruthJSON` files for each attack must be merged into two files: `merged_traceJSON.parquet` and `merged_traceGroundTruth.parquet`.
To perform the merge, run the `merge_parquet_per_attack.sh` script as follows:

```console
# ./merge_parquet_per_attack.sh <folder/containig/parquet/files> <num_cores>
./merge_parquet_per_attack.sh ../Data $(nproc)
```

This will generate, inside each attack folder, a `merged_traceJSON.parquet` file and a `merged_traceGroundTruth.parquet` file. These files will contain, respectively, all the `traceJSON` and `traceGroundTruthJSON` parquet files merged for that specific attack.


## Label data

Once merged JSON files have been created, logged messages can be checked against their ground-truth and consequently labeled.
To do so select one folder containg one merged-ground-truth file and launch the bash script `label_all_veremi_folders.sh` like in this example:

```console
# ./label_all_veremi_folders.sh <folder/containing/merged/files> <num_cores>
./label_all_veremi_folders.sh ../Data 20
```

This will generate in the selected folder a `{attackName}_labeledSummary.parquet` that is the list of all messages logged by each receiver labeled if they were detected as genuine or malicious during the labeling process.


## Preparing sequences

The `{attackName}_labeledSummary.parquet` files should be further processed/splitted in order to build the input vectors used for feeding the neural network.

To do so:

```console
./sequence_all_veremi_folders.sh <top/folder/with/{attackName}_labeledSummary.parquet> <WINDOW-WIDTH> <WINDOW-STYLE> <labelingPolicy> <NUM_CORES>
```

e.g.

```console
./sequence_all_veremi_folders.sh ../Data 5 jumping last 20
```

This will generate, inside each attack folder, two parquet files: `xsequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet` and `ysequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet`.


## Merge sequences

Once `xsequences*.parquet` and `ysequences*.parquet` files have been created inside each attack folder, all the `xsequences*.parquet` files and all the `ysequences*.parquet` files must be merged in two files.

To do so run the script `merge_all_sequences.sh` like in this example:

```console
# ./merge_all_sequences.sh <path/to/(x|y)sequences/files> 
./merge_all_sequences.sh "../Data/*/xsequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet"
```

This will generate, in the folder you launched the script, two files: `ALL_xsequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet` and `ALL_ysequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet`.