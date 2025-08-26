import pandas as pd
import sys
import code #code.interact(local=dict(globals(), **locals()))
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from itertools import product

plt.rcParams['text.usetex'] = True
plt.rcParams['axes.grid'] = True

CRASHEDCSVFILE = sys.argv[1]

outname = CRASHEDCSVFILE.replace(".csv", ".pdf")

NUMEXPxMISTYPEandDef = 300



labels = {"constPos": r"$C_{pos}$",
          "randomPos": r"$R_{pos}$",
          "randomOffset": r"$O_{pos}$",
          "randomSpeed": r"$R_{spd}$",
          "randomOffsetSpeed": r"$O_{spd}$",
          "eventualStop": r"$EvSt$",
          "disruptive": r"$Dis$",
          "dataReplay": r"$D_{rep}$"}

if "attack" in outname:
    labels = {"disruptive": r"$Dis$",
          "dataReplay": r"$D_{rep}$"}


df = pd.read_csv(CRASHEDCSVFILE)

#'results/Misbehavior4_nodef_disruptive_1_3_0.1_18_collision.xml'
df['def'] = df['file'].apply(lambda x: False if x.split('_')[1] == "nodef" else True)
df['mistype'] = df['file'].apply(lambda x: x.split('_')[2])

df = df.drop('file', axis=1)

tmp = df.groupby(["mistype", "def"]).count()/NUMEXPxMISTYPEandDef*100
tmp = tmp['lane'].reset_index()
tmp = tmp.rename(columns={"lane": "collisionRate"})


# create data
NUMLABELS=len(labels)
x = np.arange(NUMLABELS)

crNoDef = []
crWithDef = []
xlabels = []
for exp in product(labels.keys(), [True,False]):
    mis, difesa = exp
    data = tmp[(tmp.mistype == mis) & (tmp['def']==difesa)]
    cr = 0 if data.empty else data.collisionRate.iloc[0]
    if (difesa):
        crWithDef.append(cr)
    else:
        crNoDef.append(cr)
        xlabels.append(labels[mis])


width = 0.3

# plot data in grouped manner of bar type
plt.figure(figsize=(4,2.35))
plt.bar(x-0.3, crNoDef, width, edgecolor='k', color='#e41a1c', zorder=2, label="W/O Defense")
plt.bar(x, crWithDef, width, edgecolor='k', color='#377eb8', zorder=2, label="With Defense")

plt.xticks(x,labels=xlabels, fontsize=8, rotation=45)
plt.grid(axis='y',zorder=0)
plt.ylabel("Collision Rate")
ax = plt.gca()
from matplotlib.ticker import FuncFormatter
ax.yaxis.set_major_formatter(FuncFormatter(lambda y, _: f'{int(y)}\\%'))
plt.xlabel("Misbehaviors")
plt.legend(bbox_to_anchor=(0.18, 0.99), ncol=3, fontsize=7)

if "attack" in outname:
    plt.legend(bbox_to_anchor=(0.78, 1.17), ncol=3, fontsize=7)


plt.subplots_adjust(left=0.16, bottom=0.24, right=0.98, top=0.9)
plt.savefig(f"collRate_{outname}", format='pdf')