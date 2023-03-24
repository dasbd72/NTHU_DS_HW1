import sys
from sortedcontainers import SortedDict

argc = len(sys.argv)
try:
    assert(argc == 3)
except:
    print('diff: error: wrong input format')
    print('input format: python diff.py {INPUT_FILE_PATH} {OUTPUT_FILE_PATH}')
    exit(-1)

INPUT_FILE_PATH = sys.argv[1]
OUTPUT_FILE_PATH = sys.argv[2]

with open(INPUT_FILE_PATH) as file:
    input_list = file.readlines()

with open(OUTPUT_FILE_PATH) as file:
    output_list = file.readlines()

def parse(list):
    ret = SortedDict()
    for line in list:
        trxn, freq = line.split(':')
        trxn = tuple(sorted(map(int, trxn.split(','))))
        freq = float(freq)
        assert(trxn not in ret.keys())
        ret[trxn] = freq
    return ret

output = parse(output_list)
ans = parse(ans_list)

assert(output == ans)