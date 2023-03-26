# FP growth

## Spec

* Input file:
  * Lines of transactions
  * Each item in transaction is separated by comma
  * Newline is '\n'
* Output file:
  * {Frequent pattern}:{Support}
* Execute command:
  * ./109062131_hw1 {min_support} {input_filename} {output_filename}
* Judge machine:
  * CPU: i7-8700k
  * RAM: 32G
  * OS: Ubuntu 20.04.3 LTS
  * gcc vesion: 11.3.0

## Implementation

An fpgrowth implementation.

### Optimizations

* single path
  * maintains the single path variable when building tree
* data structure
  * use hash table (unordered_map) for header table and item frequency storage
* output optimization
  * buffer output lines in ostringstream
  * **write to file when buffer size reaches *MAXOSSBUF***
    * the most important optimization
* parallelization
  * since overhead is the process finding combinations
  * parallelized from the branches of combination tree
  * each thread starts from one branch of combination

##

## Usage

### Compile main program

```bash
make clean && make all
```

### Compile scripts

```bash
cd scripts
make clean && make all
```

### unzip testcases

```bash
sudo apt install unzip
cd testcases
unzip testcases
```

### fast execute script

```bash
mkdir outputs
bash scripts/exec.sh {min_support} {testcase}
```

## Performance

* config
  * ubuntu server 22.04 LTS vm under proxmox
    * cpu: intel 13400 host 6 cores
    * mem: 24G
  * MAXOSSBUF (256 * 1024)
  * command: `bash scripts/exec.sh 0.2 01`
* 1c: 14.1459s
* 2c: 8.13519s
* 3c: 7.66031s
* 4c: 4.5006s
* 5c: 4.69443s
* 6c: 5.30194s
