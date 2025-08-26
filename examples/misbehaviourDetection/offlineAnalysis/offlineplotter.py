from sklearn.metrics import classification_report
#from keras.metrics import Precision, Recall, Metric
import matplotlib.pyplot as plt
import seaborn as sns
from sklearn.metrics import confusion_matrix
import pickle
import pandas as pd
import numpy as np
import sys
from matplotlib.ticker import FuncFormatter
import code #code.interact(local=dict(globals(), **locals()))

plt.rcParams['text.usetex'] = True

# Metrics
metrics = ['precision', 'recall', 'f1-score']
labels = [r"$gen$", r"$C_{pos}$", r"$R_{pos}$", r"$O_{pos}$", r"$R_{spd}$", r"$O_{spd}$", r"$EvSt$", r"$Dis$", r"$D_{rep}$"]
binlabels = [r"$gen$", r"$mal$"]
# colors = ['#66c2a5','#fc8d62','#8da0cb']
colors = ['#8dd3c7','#ffffb3','#bebada']

blues = ['#deebf7','#9ecae1','#3182bd']
reds = ['#fee0d2','#fc9272','#de2d26']
greens = ['#e5f5e0','#a1d99b','#31a354']


############################################
# Load plottable data

df = pd.read_parquet(sys.argv[1])

df['mdsjoinpl'] = df.apply(lambda row: int(row.tl) if row.mdspl == 1 else 0, axis=1)
df['bl'] = df.tl.apply(lambda x: 0 if x == 0 else 1)

################################
# Accuracy BarChart
################################

def accuracyBarChart(yt, yp, prefix, colors=colors, NUMLABELS=9):

    mask = ~np.isnan(yp)  # True where y_pred is not NaN
    y_pred_clean = yp[mask]
    y_true_clean = yt[mask]

    print(f"NUM UNFILTERED SAMPLES: {len(yp)}")
    print(f"NUM SAMPLES for plotting: {len(y_pred_clean)}\n"+'-'*25)

    report = classification_report(y_true_clean, y_pred_clean, output_dict=True)
    rdf = pd.DataFrame(report).transpose()
    rdf = rdf.iloc[0:NUMLABELS,:-1]

    lbls = labels
    legendXOffset = 0
    legendYOffset = 0
    if NUMLABELS == 2:
        lbls = binlabels
        legendXOffset = 0.7
        legendYOffset = 0.16


    # create data
    x = np.arange(NUMLABELS)
    yprec = rdf.precision
    yrec = rdf.recall
    yf1 = rdf['f1-score']
    width = 0.26

    # plot data in grouped manner of bar type
    plt.figure(figsize=(4,2.35))
    plt.bar(x-0.26, yprec, width, edgecolor='k', color=colors[0], zorder=2, label="Precision")
    plt.bar(x, yrec, width, edgecolor='k', color=colors[1], zorder=2, label="Recall")
    plt.bar(x+0.26, yf1, width, edgecolor='k', color=colors[2], zorder=2, label="F1-Score")

    plt.xticks(x,labels=lbls, fontsize=8, rotation=45)
    plt.grid(axis='y',zorder=0)
    plt.ylim(0.7,1.01)
    plt.ylabel("Accuracy")
    plt.xlabel("Message Types")
    plt.legend(bbox_to_anchor=(legendXOffset+0.18, legendYOffset+0.99), ncol=3, fontsize=7)
    plt.subplots_adjust(left=0.12, bottom=0.24, right=0.98, top=0.82)
    plt.savefig(prefix+"_accuracyBarChart.pdf", format='pdf')
    print(f"- Saved {prefix}_accuracyBarChart.pdf")
    plt.clf()

print("JOIN:")
accuracyBarChart(df.tl, df.mdsjoinpl, "joint", colors=greens)

print("AI:")
accuracyBarChart(df.tl, df.nblai, "AI", colors=reds)

# For RULE use BinaryLabel (bl) as ground truth
print("RULE:")
accuracyBarChart(df.bl, df.lr, "RULE", NUMLABELS=2, colors=blues)

################################
# Conf matrix
################################

def confMatrix(yt, yp, prefix, cmap='magma'):

    mask = ~np.isnan(yp)  # True where y_pred is not NaN
    yp = yp[mask]
    yt = yt[mask]
    cm = confusion_matrix(yt, yp, normalize='true')

    lbls = labels
    divider = 1
    if (len(yt.unique())==2):
        lbls=binlabels
        divider = 1.2


    plt.figure(figsize=(6/divider,3.38))
    fmt = lambda x,pos:'{:.0%}'.format(x)
    ax = sns.heatmap(cm, annot=True, fmt='.1%', cmap=cmap,
     xticklabels=lbls, yticklabels=lbls)
    colorbar = ax.collections[0].colorbar
    colorbar.set_ticks([0, 0.2, 0.4, 0.6, 0.8, 1.0])
    colorbar.set_ticklabels([r'0\%', r'20\%', r'40\%', r'60\%', r'80\%', r'100\%'])
    colorbar.set_label('Recall', rotation=90, fontsize=12, labelpad=-52)

    plt.xticks(rotation=30, fontsize=14)
    plt.yticks(fontsize=14)
    plt.xlabel('Predicted label', fontsize=14)
    plt.ylabel('True label', fontsize=14)
    plt.subplots_adjust(left=0.13, bottom=0.24, right=1.04, top=0.91)
    plt.savefig(prefix+"_conf_offline.pdf", format='pdf')
    print(f"- Saved {prefix}_conf_offline.pdf")


# confMatrix(df.tl, df.mdsjoinpl, "joint", cmap='Greens')
# confMatrix(df.tl, df.nblai, "AI", cmap='Reds')
# # # For RULE use BinaryLabel (bl) as ground truth
# confMatrix(df.bl, df.lr, "RULE", cmap='Blues')