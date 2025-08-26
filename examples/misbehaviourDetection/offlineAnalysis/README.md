## Evaluating AI and RULE MDS offline

The Hybrid MDS is first evaluated offline on the VeReMi dataset by running both the AI-based and the rule-based methods on the same sequence of messages in the dataset.
To do so, run:

```console
python pevalmds_offline.py -j <NUM_PARALLEL_JOBS> \
    --dataFraction <DATA_FRACTION> \
    --model $MISB/NNtraining/training/model.keras \
    --scaler $MISB/NNtraining/training/scaler.pkl \
    --MCdropRep <MC_DROPREP> \
    --art
```

e.g.

```console
python pevalmds_offline.py -j 8 \
    --dataFraction 0.1 \
    --model $MISB/NNtraining/training/lstmModel.keras \
    --scaler $MISB/NNtraining/training/stdScaler.pkl \
    --MCdropRep 10 \
    --art
```

Where:
* ```--dataFraction``` represents the fraction of the dataset to be evaluated;
* ```--MCdropRep``` specifies the number of Monte Carlo Dropout forward passes;
* ```--art``` is an optional flag to enable an additional rule-based check during the evaluation.

The output of ```pevalmds_offline.py``` script is a ```parquet``` file saved as ```mds_datafr_<DATA_FRACTION*100>_MCdropRep_<MC_DROPREP>_MCdropCU10_wART.parquet``` (with ```wART``` in the filename if the ```--art``` flag was specified).

This file is passed as argument of the ```offlineplotter.py``` script to produce the accuracy bar chart (Precision, Recall and F1-Score metrics) and the confusion matrix:

```console
python offlineplotter.py mds_datafr_<DATA_FRACTION*100>_MCdropRep_<MC_DROPREP>_MCdropCU10_wART.parquet
```

The file is also passed as argument of the ```score_distrib.py``` script to produce the boxplots of the decider score for all message types:

```console
python score_distrib.py mds_datafr_<DATA_FRACTION*100>_MCdropRep_<MC_DROPREP>_MCdropCU10_wART.parquet
```
