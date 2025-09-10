## Training AI model

Once produced the `xsequences` and `ysequences` in preprocessing step, to train the model make sure to be in the folder `examples/misbehaviourDetection/NNtraining/training` and digit

```console
python newtrain.py --labels ../preprocessing/ALL_ysequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet \
  --data ../preprocessing/ALL_xsequences_ww_<WINDOW-WIDTH>_ws_<WINDOW-STYLE>_policy_<labelingPolicy>.parquet \
  --num_steps <WINDOW-WIDTH - 1> \
  --num_features 6 \
  --class_weight_strategy <classWeightStrategy>
```

This will generate, in the folder you launched the script, three files: `model.keras`, `standard_scaler.pkl` and `plottable.pkl`.
The last one is useful to plot metrics and learning rate with `plotter.py` script passing this file as argument.
The first two, instead, are used for the following linking step.

## Linking

Now, in `examples/misbehaviourDetection` folder, modify the `omnetpp.ini` file to correctly link the model and the StandardScaler produced

```c++
*.node[*].appl.model_path = "./NNtraining/training/model.keras"
*.node[*].appl.scaler_path = "./NNtraining/training/standard_scaler.pkl"
*.node[*].appl.window_style = "sliding" // or "jumping"
```