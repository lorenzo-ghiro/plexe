import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import code  # code.interact(local=dict(globals(), **locals()))

plt.rcParams['axes.grid'] = True
plt.rcParams["text.usetex"] = True

plt.rcParams['text.latex.preamble'] = r'\usepackage{amsmath} \usepackage{amsfonts}'

# Metrics
metrics = ['precision', 'recall', 'f1-score']
labels = [r"$gen$", r"$C_{pos}$", r"$R_{pos}$", r"$O_{pos}$",
          r"$R_{spd}$", r"$O_{spd}$", r"$EvSt$", r"$Dis$", r"$D_{rep}$"]
binlabels = [r"$gen$", r"$mal$"]

colors = ['whitesmoke', '#8dd3c7', '#ffffb3', '#bebada', '#fb8072', '#80b1d3', '#fdb462', '#b3de69', '#fccde5']
custom_palette = sns.color_palette(colors)


# to correct wrong computation made during experiments
def CSF(lr, cr, lai, cai):
    rterm = 0 if np.isnan(lr) else cr * (2 * lr - 1)
    aiterm = cai * (2 * lai - 1)
    return rterm + aiterm

# colors = ['#8dd3c7','#ffffb3','#bebada']

# blues = ['#deebf7','#9ecae1','#3182bd']
# reds = ['#fee0d2','#fc9272','#de2d26']
# greens = ['#e5f5e0','#a1d99b','#31a354']


df = pd.read_parquet(sys.argv[1])

df['mdso'] = df.apply(lambda row: CSF(row.lr, row.cr, row.lai, row.cai), axis=1)

outname = sys.argv[1].split('/')[-1].replace(".parquet", "")

plt.figure(figsize=(4, 2.5))

# medianprops = dict(linestyle='-', linewidth=2.5, color='firebrick')
flierprops = dict(marker='x', markerfacecolor='k',
                  markersize=1.2, markeredgecolor='k')
meanlineprops = dict(linestyle='-', linewidth=2.5, color='firebrick')

sns.boxplot(data=df, x="tl", y="norm_sum", linewidth=1, whis=(5, 95), palette=custom_palette,
            flierprops=flierprops, meanprops=meanlineprops, meanline=True, showmeans=True)
# Strip plot (punti individuali jittered)
# sns.stripplot(data=df, x="tl", y="norm_sum", color="black", alpha=0.3, jitter=0.2, size=2)

plt.xlabel("Message Type")
plt.xticks(range(0, 9), labels=labels, fontsize=8, rotation=45)
plt.ylabel(r'$s_\text{{\tiny{R}}}$', fontsize=14)
SOGLIA = 1/4 if "wART" in outname else 1/3
plt.axhline(y=SOGLIA, linewidth=1.12, color='r', ls='--')
plt.tight_layout()
plt.yticks(np.arange(0, 0.76, 0.25))
plt.savefig(f"sumscoredistrib_{outname}.png", dpi=300)


##
# FUSED SCORE DISTRIB
##

plt.figure(figsize=(4, 2.5))

sns.boxplot(data=df, x="tl", y="mdso", linewidth=1, whis=(5, 95), palette=custom_palette,
            flierprops=flierprops, meanprops=meanlineprops, meanline=True, showmeans=True)

# Strip plot (punti individuali jittered)
# sns.stripplot(data=df, x="tl", y="norm_sum", color="black", alpha=0.3, jitter=0.2, size=2)

plt.xlabel("Message Type")
plt.xticks(range(0, 9), labels=labels, fontsize=8, rotation=45)
plt.ylabel(r"$s_d$", fontsize=14)
plt.axhline(y=0.0, linewidth=1.12, color='r', ls='--')
plt.tight_layout()
# plt.yticks(np.arange(0, 0.76, 0.25))
plt.savefig(f"fused_score_distrib{outname}.png", dpi=300)
