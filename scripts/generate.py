import csv
import sys
import numpy as np
from tqdm import tqdm

argc = len(sys.argv)
try:
    assert(argc == 2)
except:
    print('[Error] input format: python generate.py {INPUT_FILE_PATH}')
    exit(-1)

MAX_NUM_TRXN = 100000
MAX_ITEM = 1000
MAX_LEN_TRXN = 200

INPUT_FILE_PATH = sys.argv[1]

NUM_TRXN = MAX_NUM_TRXN
ITEM = MAX_ITEM
LEN_TRXN = MAX_LEN_TRXN

assert(NUM_TRXN <= MAX_NUM_TRXN)
assert(ITEM <= MAX_ITEM)
assert(LEN_TRXN <= MAX_LEN_TRXN)

candidates = np.arange(ITEM)
weights = np.random.rand(ITEM)
weights /= weights.sum()
probs = np.random.dirichlet(weights)

with open(INPUT_FILE_PATH, mode='w', newline='') as file:
    writer = csv.writer(file)
    for _ in tqdm(range(NUM_TRXN)):
        trxn = np.random.choice(candidates, size=int(np.random.randint(1,LEN_TRXN+1)), replace=False, p=probs)
        writer.writerow(trxn)
