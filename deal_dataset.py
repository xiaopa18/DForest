# coding=utf-8
import os.path
import time

import pandas as pd
import numpy as np
from sklearn.decomposition import PCA
from functions import to_string
import joblib
import sys

dir = './data_set/'+dataid+'/'
file_name = dataid+'.csv'

argv = sys.argv
print(argv)
if len(argv) != 2 and len(argv) != 1:
    print("parameter error")
    exit(-1)
if len(argv) != 1:
    dataid=argv[1]
    dir = './data_set/'+dataid+'/'
    file_name = dataid+'.csv'

print(dir+file_name)
dataset = pd.read_csv(dir+file_name,dtype=np.float32,header=None).dropna(axis=1,how='all').to_numpy()
save_file=dir+'model'

pca = PCA(0.9999999)

fit_time = -time.time()
pca.fit(dataset)
fit_time += time.time()
print('fit model over')
joblib.dump(pca,save_file)

rt=pca.explained_variance_ratio_

with open(dir+'baoliu_info.csv','w') as f:
    sm=0.0
    for i in range(len(rt)):
        sm+=rt[i]
        f.write(str(i+1)+','+str(sm)+'\n')

tim = -time.time()
dataset = pca.transform(dataset)
tim += time.time()

np.savetxt(dir+file_name.split('.')[0]+'_afterpca.csv',dataset,delimiter=',',fmt='%f')
print(f'model fit time: {fit_time*1000}ms')
print(f'data transform time: {tim*1000}ms')
