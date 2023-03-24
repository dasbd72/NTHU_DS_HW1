import sys
from tqdm import tqdm
from sortedcontainers import SortedDict

argc = len(sys.argv)
try:
    assert(argc == 3)
except:
    print('diff: error: wrong input format')
    print('input format: python diff.py {OUTPUT_FILE_PATH} {ANS_FILE_PATH}')
    exit(-1)

OUTPUT_FILE_PATH = sys.argv[1]
ANS_FILE_PATH = sys.argv[2]

with open(OUTPUT_FILE_PATH) as file:
    output_list = file.readlines()
with open(ANS_FILE_PATH) as file:
    ans_list = file.readlines()

def parse(list):
    # ret = SortedDict()
    # for line in tqdm(list):
    #     trxn, freq = line.split(':')
    #     trxn = tuple(sorted(map(int, trxn.split(','))))
    #     freq = float(freq)
    #     assert(trxn not in ret.keys())
    #     ret[trxn] = freq
    # return ret
    ret = []
    for line in tqdm(list):
        trxn, freq = line.split(':')
        trxn = tuple(sorted(map(int, trxn.split(','))))
        freq = float(freq)
        ret.append((trxn, freq))
    ret.sort()
    return ret

output = parse(output_list)
ans = parse(ans_list)

assert(output == ans)