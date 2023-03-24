#include <omp.h>
#include <pthread.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <vector>

#ifdef DEBUG
#define DEBUG_MSG(str) std::cout << str << "\n";
#else
#define DEBUG_MSG(str)
#endif  // DEBUG

#ifdef TIMING
#include <ctime>
#define TIMING_START(arg)          \
    struct timespec __start_##arg; \
    clock_gettime(CLOCK_MONOTONIC, &__start_##arg);
#define TIMING_END(arg)                                                                       \
    {                                                                                         \
        struct timespec __temp_##arg, __end_##arg;                                            \
        double __duration_##arg;                                                              \
        clock_gettime(CLOCK_MONOTONIC, &__end_##arg);                                         \
        if ((__end_##arg.tv_nsec - __start_##arg.tv_nsec) < 0) {                              \
            __temp_##arg.tv_sec = __end_##arg.tv_sec - __start_##arg.tv_sec - 1;              \
            __temp_##arg.tv_nsec = 1000000000 + __end_##arg.tv_nsec - __start_##arg.tv_nsec;  \
        } else {                                                                              \
            __temp_##arg.tv_sec = __end_##arg.tv_sec - __start_##arg.tv_sec;                  \
            __temp_##arg.tv_nsec = __end_##arg.tv_nsec - __start_##arg.tv_nsec;               \
        }                                                                                     \
        __duration_##arg = __temp_##arg.tv_sec + (double)__temp_##arg.tv_nsec / 1000000000.0; \
        std::cout << #arg << " took " << __duration_##arg << "s.\n";                          \
        std::cout.flush();                                                                    \
    }
#else
#define TIMING_START(arg)
#define TIMING_END(arg)
#endif  // TIMING

#define MAXFREQ 20000000
#define MAXTRXNS 100000
#define MAXTRXN 200
#define MAXITEM 1000
#define NULLITEM -1
#define REFINC(_ref) [&](int x, int y) { \
    if (_ref[x] != _ref[y])              \
        return _ref[x] < _ref[y];        \
    else                                 \
        return x > y;                    \
}
#define REFDEC(_ref) [&](int x, int y) { \
    if (_ref[x] != _ref[y])              \
        return _ref[x] > _ref[y];        \
    else                                 \
        return x < y;                    \
}

using Item = int;
using Transaction = std::vector<Item>;
using Pattern = std::pair<Transaction, int>;

struct FPNode {
    // Metadata
    Item item;
    int cnt;
    // link list data (same item)
    FPNode* next;
    // tree data
    FPNode* parent;
    std::map<Item, FPNode*> child;

    FPNode(Item item) : item(item), cnt(0), next(NULL), parent(NULL) {}
};
struct FPTree {
    FPNode* root;
    std::array<FPNode*, MAXITEM> hdr_table;
    std::vector<Item> items_by_freq;      // items above minimum support, sorts decreasing by freq
    std::array<Item, MAXITEM> item_freq;  // item frequency count

    FPTree() {
        item_freq.fill(0);
        root = new FPNode(-1);
        for (int i = 0; i < MAXITEM; i++) {
            hdr_table[i] = new FPNode(-1);
        }
    }
    ~FPTree() {
        deleteTree(root);
        for (int i = 0; i < MAXITEM; i++) {
            delete hdr_table[i];
        }
    };

    bool empty();
    void fpgrowth(const Transaction& base, const int& min_sup, std::vector<Pattern>& fplist);

    bool hasSinglePath();
    // Debug use, output traverse of fptree
    void traverse(FPNode* node);
    // Delete subtree from node
    void deleteTree(FPNode* node);
};

void read_transactions(const std::string& input_filename, std::vector<Transaction>& trxns, FPTree& fptree);
// Define a function to output a single pattern to the output stream
void output_pattern(std::ostringstream& oss, const Pattern& fp, size_t trxns_size);
// Define a function to output the list of patterns to the output file
void output_patterns(const std::string& output_filename, const std::vector<Pattern>& fplist, size_t trxns_size);

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    assert(argc == 4);
    double fmin_sup = atof(argv[1]);
    std::string input_filename = argv[2];
    std::string output_filename = argv[3];
    int n_cpus = omp_get_max_threads();

    DEBUG_MSG("Cpus: " << n_cpus);

    std::vector<Transaction> trxns;  // transaction list
    int min_sup;
    FPTree fptree;
    std::vector<Pattern> fplist;  // frequent patterns

    // input
    // build trxns and item_freq
    // first scan, count freq and load transactions
    TIMING_START(input);
    read_transactions(input_filename, trxns, fptree);
    TIMING_END(input);

    // construct fptree
    // set min_sup, build fptree
    // second scan, build fptree and update table
    TIMING_START(build_tree);
    {
        min_sup = ceil(fmin_sup * trxns.size());  // transform min support percent to min support count

        for (int i = 0; i < MAXITEM; i++) {
            if (fptree.item_freq[i] >= min_sup) {
                fptree.items_by_freq.emplace_back(i);
            }
        }
        std::sort(fptree.items_by_freq.begin(), fptree.items_by_freq.end(), REFDEC(fptree.item_freq));

        std::array<FPNode*, MAXITEM> tail_table(fptree.hdr_table);
        for (int i = 0; i < (int)trxns.size(); i++) {
            // transaction with frequent items
            Transaction trxn;
            for (int j = 0; j < (int)trxns[i].size(); j++) {
                Item item = trxns[i][j];
                if (fptree.item_freq[item] >= min_sup)
                    trxn.emplace_back(item);
            }
            // sort transaction by freq
            std::sort(trxn.begin(), trxn.end(), REFINC(fptree.item_freq));

            // add path to fptree
            FPNode* curr = fptree.root;
            for (int j = (int)trxn.size() - 1; j >= 0; j--) {
                Item item = trxn[j];
                auto it = curr->child.find(item);
                // if exist move curr, else create and move
                if (it != curr->child.end()) {
                    curr = it->second;
                } else {
                    FPNode* node = new FPNode(item);
                    node->parent = curr;
                    curr = tail_table[item] = curr->child[item] = tail_table[item]->next = node;
                }
                curr->cnt++;
            }
        }
    }
    TIMING_END(build_tree);

    // fpgrowth
    TIMING_START(fpgrowth);
    fptree.fpgrowth(Transaction(), min_sup, fplist);
    TIMING_END(fpgrowth);

    // output
    TIMING_START(output);
    output_patterns(output_filename, fplist, trxns.size());
    TIMING_END(output);
}

bool FPTree::empty() {
    return root->child.size() == 0;
}

void FPTree::fpgrowth(const Transaction& base, const int& min_sup, std::vector<Pattern>& fplist) {
    if (hasSinglePath()) {
        std::vector<Pattern> subfplist;
        for (FPNode* curr = root; !curr->child.empty();) {
            curr = curr->child.begin()->second;
            for (int i = (int)subfplist.size() - 1; i >= 0; i--) {
                auto new_pattern = subfplist[i];
                new_pattern.first.emplace_back(curr->item);
                new_pattern.second = curr->cnt;
                subfplist.emplace_back(new_pattern);
            }
            subfplist.emplace_back(Transaction(1, curr->item), curr->cnt);
        }

        if (!base.empty()) {
            for (auto& pattern : subfplist) {
                pattern.first.insert(pattern.first.end(), base.begin(), base.end());
            }
        }
        fplist.insert(fplist.end(), subfplist.begin(), subfplist.end());
    } else {
        for (int i = (int)items_by_freq.size() - 1; i >= 0; i--) {
            Item baseItem = items_by_freq[i];
            FPTree fptree;  // the conditional fptree
            std::vector<Pattern> subfplist;

            // count frequency of current tree
            for (FPNode* leaf = hdr_table[baseItem]->next; leaf != NULL; leaf = leaf->next) {
                for (FPNode* curr = leaf->parent; curr != root; curr = curr->parent) {
                    fptree.item_freq[curr->item] += leaf->cnt;
                }
            }
            for (int j = 0; j < MAXITEM; j++) {
                if (fptree.item_freq[j] >= min_sup)
                    fptree.items_by_freq.emplace_back(j);
            }
            std::sort(fptree.items_by_freq.begin(), fptree.items_by_freq.end(), REFDEC(fptree.item_freq));

            // construct conditional tree
            std::array<FPNode*, MAXITEM> tail_table(fptree.hdr_table);
            for (FPNode* leaf = hdr_table[baseItem]->next; leaf != NULL; leaf = leaf->next) {
                // get path
                Transaction trxn;
                for (FPNode* curr = leaf->parent; curr != root; curr = curr->parent) {
                    if (fptree.item_freq[curr->item] >= min_sup)
                        trxn.emplace_back(curr->item);
                }

                // add path to conditional tree
                FPNode* curr = fptree.root;
                for (int j = (int)trxn.size() - 1; j >= 0; j--) {
                    Item item = trxn[j];
                    auto it = curr->child.find(item);
                    // if exist move curr, else create and move
                    if (it != curr->child.end()) {
                        curr = it->second;
                    } else {
                        FPNode* node = new FPNode(item);
                        node->parent = curr;
                        curr = tail_table[item] = curr->child[item] = tail_table[item]->next = node;
                    }
                    curr->cnt += leaf->cnt;
                }
            }

            Transaction sub_base = base;
            sub_base.emplace_back(baseItem);
            fplist.emplace_back(sub_base, item_freq[baseItem]);
            if (!fptree.empty()) {
                fptree.fpgrowth(sub_base, min_sup, fplist);
            }
        }
    }
}

bool FPTree::hasSinglePath() {
    for (FPNode* curr = root; !curr->child.empty(); curr = curr->child.begin()->second) {
        if (curr->child.size() > 1)
            return false;
    }
    return true;
}

void FPTree::traverse(FPNode* node) {
    if (node == NULL)
        return;
    for (auto child : node->child) {
        std::cout << child.second->item << " " << child.second->cnt << "\n";
        traverse(child.second);
    }
}

void FPTree::deleteTree(FPNode* node) {
    if (node == NULL)
        return;
    for (auto child : node->child)
        deleteTree(child.second);
    delete node;
}

void read_transactions(const std::string& input_filename, std::vector<Transaction>& trxns, FPTree& fptree) {
    std::ifstream input_file(input_filename);
    std::string str;
    trxns.reserve(MAXTRXNS);
    while (getline(input_file, str)) {
        Transaction trxn;
        trxn.reserve(MAXTRXN);
        size_t start = 0;
        size_t end = str.find(',');

        while (true) {
            Item item = std::stoi(str.substr(start, end - start));
            trxn.emplace_back(item);
            fptree.item_freq[item]++;
            if (end == std::string::npos)
                break;
            start = end + 1;
            end = str.find(',', start);
        }
        trxns.emplace_back(trxn);
    }
    input_file.close();
}

// Define a function to output a single pattern to the output stream
void output_pattern(std::ostringstream& oss, const Pattern& fp, size_t trxns_size) {
    oss << fp.first[0];
    std::for_each(fp.first.begin() + 1, fp.first.end(), [&](const auto& item) {
        oss << ',' << item;
    });
    oss << std::fixed << std::setprecision(4) << ':' << (double)fp.second / trxns_size << '\n';
}

// Define a function to output the list of patterns to the output file
void output_patterns(const std::string& output_filename, const std::vector<Pattern>& fplist, size_t trxns_size) {
    // Use ostringstream to buffer the output before writing to file
    std::ostringstream oss;
    oss.precision(4);
    std::for_each(fplist.begin(), fplist.end(), [&](const auto& fp) {
        output_pattern(oss, fp, trxns_size);
    });

    // Open output file and write the buffered output to it
    std::ofstream output_file(output_filename);
    if (output_file.is_open()) {
        auto output_str = oss.str();
        output_file.write(output_str.data(), output_str.size());
        output_file.close();
    }
}