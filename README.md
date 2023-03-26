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
