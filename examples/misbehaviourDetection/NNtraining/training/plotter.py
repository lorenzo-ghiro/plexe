from sklearn.metrics import classification_report
from keras.metrics import Precision, Recall, Metric
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import confusion_matrix
import pickle
import pandas as pd
import numpy as np
import sys
import os
import code #code.interact(local=dict(globals(), **locals()))

plt.rcParams['text.usetex'] = True

# Metrics
metrics = ['precision', 'recall', 'f1-score']
labels = [r"$gen$", r"$C_{pos}$", r"$R_{pos}$", r"$O_{pos}$", r"$R_{spd}$", r"$O_{spd}$", r"$EvSt$", r"$Dis$", r"$D_{rep}$"]
binlabels = ["Genuine", "Malicious"]

############################################
# Load plottable data

PLOTTABLE_FILE = sys.argv[1]
saveas = PLOTTABLE_FILE.split("/")[-1].replace("plottable_", "").replace(".pkl", "")

f = open(PLOTTABLE_FILE, 'rb')
history, yt, yp = pickle.load(f)

report = classification_report(yt, yp, output_dict=True)
rdf = pd.DataFrame(report).transpose()
rdf = rdf.iloc[0:9,:-1]

# create data
NUMLABELS=9
x = np.arange(NUMLABELS)
yprec = rdf.precision
yrec = rdf.recall
yf1 = rdf['f1-score']
width = 0.2


os.makedirs("figures", exist_ok=True)

######################################################################################
# PLOT DATA IN GROUPED MANNER OF BAR TYPE
######################################################################################
plt.figure(figsize=(4,3))
plt.bar(x-0.3, yprec, width)
plt.bar(x-0.1, yrec, width)
plt.bar(x+0.1, yf1, width)

plt.xticks(x,labels=labels, fontsize=7, rotation=45)
plt.grid(axis='y')
plt.ylim(0.7,1.01)
plt.ylabel("Accuracy Metrics")
plt.xlabel("Misbehaviors")
plt.savefig(f"figures/accuracyBarChart_{saveas}.pdf", format='pdf')
######################################################################################



######################################################################################
# ACCURACY TRAINING
######################################################################################
# acc = history.history['accuracy']
# val_acc = history.history['val_accuracy']
# epochs = range(1, len(acc) + 1)

# plt.figure(figsize=(10,6))
# plt.plot(epochs, acc, '-', label='Training Accuracy', color='blue', alpha=0.5)
# plt.plot(epochs, val_acc, '-', label='Validation Accuracy', color='red', alpha=0.5)
# plt.subplots_adjust(bottom=0.2)  # Adjust this value as needed
# plt.title('Training vs Validation Accuracy', fontsize=20)
# plt.legend()
# plt.xlabel('Epochs', fontsize=14)
# plt.ylabel('Accuracy', fontsize=14)
# plt.grid()
# plt.tight_layout()
# plt.savefig(f"figures/learning_accuracy_{saveas}.pdf")
######################################################################################



######################################################################################
# METRICS
######################################################################################
# for metric in metrics:
#     plt.figure(figsize=(12,6))
#     rdf[metric].plot(kind='bar', color=['lightgrey'], edgecolor='black')
#     plt.title(f'Class-wise {metric.capitalize()}', fontsize=20)
#     plt.subplots_adjust(bottom=0.2)  # Adjust this value as needed
#     plt.xlabel('Labels', fontsize=14)
#     plt.xticks(ticks=range(len(labels)),labels=labels, fontsize=14)
#     plt.ylabel(metric.capitalize(), fontsize=14)
#     plt.ylim(0, 1)
#     plt.xticks(rotation=45)
#     plt.grid(axis='y')
#     plt.subplots_adjust(bottom=0.3)
#     plt.savefig("figures/metric_"+metric+f"_{saveas}.pdf")
######################################################################################



######################################################################################
# CONFUSION MATRIX
######################################################################################
cm = confusion_matrix(yt, yp, normalize='true')
plt.figure(figsize=(12,8))
sns.heatmap(cm, annot=True, fmt='.4f', cmap='Reds', xticklabels=labels, yticklabels=labels)
plt.subplots_adjust(bottom=0.3, left=0.2)  # Adjust this value as needed
plt.xticks(rotation=45, fontsize=14)
plt.yticks(fontsize=14)
plt.xlabel('Predicted label', fontsize=14)
plt.ylabel('True label', fontsize=14)
plt.title('Confusion Matrix', fontsize=20)
plt.savefig(f"figures/ConfMatrix_{saveas}.pdf")
######################################################################################



######################################################################################
# Normalized binary matrix
######################################################################################
ytb, ypb = [], []
for i in range(len(yt)):
    t, p = yt[i], yp[i]
    bp = 100 if p >= 1 else 0
    bt = 100 if t >= 1 else 0
    ytb.append(bt)
    ypb.append(bp)

cm = confusion_matrix(ytb, ypb, normalize='true')
plt.figure(figsize=(12,8))
sns.heatmap(cm, annot=True, fmt='.4f', cmap='Reds', xticklabels=binlabels, yticklabels=binlabels)
plt.subplots_adjust(bottom=0.3, left=0.2)  # Adjust this value as needed
plt.xticks(rotation=45, fontsize=14)
plt.yticks(fontsize=14)
plt.xlabel('Predicted label', fontsize=14)
plt.ylabel('True label', fontsize=14)
plt.title('Binary Matrix', fontsize=20)
plt.savefig(f"figures/BinMatrix_{saveas}.pdf")
######################################################################################