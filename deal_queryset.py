# coding=utf-8
import os.path
import time

from read_function import read_csv
from sklearn.decomposition import PCA
from functions import dist,to_string
import joblib
import sys
import pandas as pd
import numpy as np

queryid='uniform1000'
dir = './data_set/'+dataid+'/'
query_file_name = f'{dataid}_{queryid}.csv'

argv = sys.argv
print(argv)
if len(argv) != 1:
    if len(argv) != 3:
        print("parameter error dataid queryid")
        exit(-1)
    dataid=argv[1]
    queryid=argv[2]
    dir = './data_set/'+dataid+'/'
    query_file_name = f'{dataid}_{queryid}.csv'



# if os.path.exists(dir+query_file_name.split('.')[0]+'_afterpca.csv'):
#     exit(0)
print(dir+query_file_name)
queryset = pd.read_csv(dir+query_file_name,dtype=np.float32,header=None).dropna(axis=1,how='all').to_numpy()
save_file = dir + 'model'
if not os.path.exists(save_file):
    print('don't have the file '+save_file)
    exit(-1)
model = joblib.load(save_file)

tim = -time.time()
queryset = model.transform(queryset)
tim += time.time()

np.savetxt(dir+query_file_name.split('.')[0]+'_afterpca.csv',queryset,delimiter=',',fmt='%f')

print(f'data ave transform time: {tim*1000/len(queryset)}ms')
