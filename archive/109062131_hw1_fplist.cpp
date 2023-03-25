#include <omp.h>
#include <pthread.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <unordered_map>
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

// tree of elements whose child frequency is always not larger
struct FPList {
    struct Node {
        int freq;
        std::unordered_map<Item, Node*> child;
        Node() : freq(-1) {}
        Node(int freq) : freq(freq) {}
        // return if current node is an end of frequent pattern
        bool isfp() const {
            return freq != -1;
        }
    };

    Node* root;

    FPList() {
        root = new Node(-1);
    }
    ~FPList() {
        deleteTree(root);
    }

    void prepend(const std::vector<Item>& lst) {
        Node* oldroot = root;
        root = new Node(-1);
        Node* curr = root;
        for (const auto& item : lst) {
            auto it = curr->child.find(item);
            if (it != curr->child.end()) {
                curr = it->second;
            } else {
                Node* node = new Node();
                curr = curr->child[item] = node;
            }
        }
        curr->child = oldroot->child;
        delete oldroot;
    }

    void insert(const std::vector<Item>& lst, int freq) {
        Node* curr = root;
        for (const auto& item : lst) {
            auto it = curr->child.find(item);
            if (it != curr->child.end()) {
                curr = it->second;
            } else {
                Node* node = new Node();
                curr = curr->child[item] = node;
            }
        }
#ifdef DEBUG
        assert(!curr->isfp());
#endif
        curr->freq = freq;
    }
    void insert(FPList& fplst) {
        recur_insert(root, fplst.root);
    }

    void recur_insert(Node* lnode, Node* rnode) {
        for (const auto& pair : rnode->child) {
            Item item = pair.first;
            Node* nxt_rnode = pair.second;
            Node* nxt_lnode;
            auto it = lnode->child.find(item);
            if (it != lnode->child.end()) {
                nxt_lnode = it->second;
            } else {
                nxt_lnode = lnode->child[item] = new Node(nxt_rnode->freq);
            }
            recur_insert(nxt_lnode, nxt_rnode);
        }
    }

    void generate(Item item, int freq) {
        generate(root, item, freq);
        root->child[item] = new Node(freq);
    }
    void generate(Node* curr, Item item, int freq) {
        for (const auto& pair : curr->child) {
            generate(pair.second, item, freq);
        }
        if (curr->isfp()) {
#ifdef DEBUG
            assert(curr->child.find(item) == curr->child.end());
#endif
            Node* node = new Node(freq);
            curr->child[item] = node;
        }
    }

    void deleteTree(Node* node) {
        for (auto& pair : node->child) {
            deleteTree(pair.second);
        }
        delete node;
    }

    void output(std::ostringstream& oss, const size_t trxns_size) const {
        std::vector<Item> buf;
        recur_output(oss, root, buf, trxns_size);
    }
    void recur_output(std::ostringstream& oss, Node* node, std::vector<Item>& buf, const size_t trxns_size) const {
        for (auto& pair : node->child) {
            buf.emplace_back(pair.first);
            if (pair.second->isfp()) {
                oss << *buf.begin();
                std::for_each(buf.begin() + 1, buf.end(), [&](const auto& item) {
                    oss << ',' << item;
                });
                oss << std::fixed << std::setprecision(4) << ':' << (double)pair.second->freq / trxns_size << '\n';
            }
            recur_output(oss, pair.second, buf, trxns_size);
            buf.pop_back();
        }
    }
};

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
    std::unordered_map<Item, FPNode*> hdr_table;
    std::vector<Item> items_by_freq;          // items above minimum support, sorts decreasing by frequency
    std::unordered_map<Item, int> item_freq;  // item frequency count

    FPTree() {
        root = new FPNode(-1);
    }
    ~FPTree() {
        deleteTree(root);
    };

    void buildFromTrxns(const std::vector<Transaction>& trxns, const int min_sup);
    void fpgrowth(const int min_sup, FPList& fplist);
    void fpgrowth(Transaction& base, int min_sup, FPList& fplist);

    bool empty();
    bool hasSinglePath();
    // Debug use, output traverse of fptree
    void traverse(FPNode* node);
    // Delete subtree from node
    void deleteTree(FPNode* node);
};

// Read transactions from file and count item frequencies
void read_transactions(const std::string& input_filename, std::vector<Transaction>& trxns, FPTree& fptree);
// Define a function to output the list of patterns to the output file
void output_patterns(const std::string& output_filename, const FPList& fplist, size_t trxns_size);

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    assert(argc == 4);
    double fmin_sup = atof(argv[1]);
    std::string input_filename = argv[2];
    std::string output_filename = argv[3];
    int n_cpus = omp_get_max_threads();

    DEBUG_MSG("Cpus: " << n_cpus);

    int min_sup;
    FPTree fptree;
    std::vector<Transaction> trxns;  // transaction list
    FPList fplist;                   // frequent patterns

    // input
    // build trxns and item_freq
    // first scan, count freq and load transactions
    TIMING_START(total);
    TIMING_START(input);
    read_transactions(input_filename, trxns, fptree);
    min_sup = ceil(fmin_sup * trxns.size());  // transform min support percent to min support count
    TIMING_END(input);

    // construct fptree
    // set min_sup, build fptree
    // second scan, build fptree and update table
    TIMING_START(build_fptree);
    fptree.buildFromTrxns(trxns, min_sup);
    TIMING_END(build_fptree);

    // fpgrowth
    TIMING_START(fpgrowth);
    fptree.fpgrowth(min_sup, fplist);
    TIMING_END(fpgrowth);

    // output
    TIMING_START(output);
    output_patterns(output_filename, fplist, trxns.size());
    TIMING_END(output);
    TIMING_END(total);

    return 0;
}

void FPTree::buildFromTrxns(const std::vector<Transaction>& trxns, const int min_sup) {
    for (int i = 0; i < MAXITEM; i++) {
        if (item_freq[i] >= min_sup) {
            items_by_freq.emplace_back(i);
        }
    }
    std::sort(items_by_freq.begin(), items_by_freq.end(), REFDEC(item_freq));

    std::unordered_map<Item, FPNode*> tail_table;
    for (int i = 0; i < (int)trxns.size(); i++) {
        Transaction trxn;
        // prune infrequent items
        for (int j = 0; j < (int)trxns[i].size(); j++) {
            Item item = trxns[i][j];
            if (item_freq[item] >= min_sup)
                trxn.emplace_back(item);
        }
        // sort transaction by freq
        std::sort(trxn.begin(), trxn.end(), REFINC(item_freq));

        // add path to fptree
        FPNode* curr = root;
        for (int j = (int)trxn.size() - 1; j >= 0; j--) {
            Item item = trxn[j];
            auto it = curr->child.find(item);
            // if exist move curr, else create and move
            if (it != curr->child.end()) {
                curr = it->second;
            } else {
                FPNode* node = new FPNode(item);
                node->parent = curr;
                curr = curr->child[item] = node;
                if (tail_table.find(item) == tail_table.end()) {
                    hdr_table[item] = tail_table[item] = node;
                } else {
                    tail_table[item] = tail_table[item]->next = node;
                }
            }
            curr->cnt++;
        }
    }
}

void FPTree::fpgrowth(const int min_sup, FPList& fplist) {
    Transaction base;
    fpgrowth(base, min_sup, fplist);
}

void FPTree::fpgrowth(Transaction& base, const int min_sup, FPList& fplist) {
    if (hasSinglePath()) {
#ifdef DEBUG
        DEBUG_MSG("Base: ");
        for (auto item : base) {
            std::cout << item << " ";
        }
        DEBUG_MSG("");
        DEBUG_MSG("item by freq: ");
        for (int i = (int)items_by_freq.size() - 1; i >= 0; i--) {
            Item item = items_by_freq[i];
            std::cout << "(" << item << "," << item_freq[item] << "), ";
        }
        DEBUG_MSG("");
#endif

        FPList subfplist;
        for (int i = (int)items_by_freq.size() - 1; i >= 0; i--) {
            Item item = items_by_freq[i];
            subfplist.generate(item, item_freq[item]);
        }
        subfplist.prepend(base);
        fplist.insert(subfplist);
    } else {
        for (int i = (int)items_by_freq.size() - 1; i >= 0; i--) {
            Item baseItem = items_by_freq[i];
            FPTree fptree;  // the conditional fptree
            std::vector<Pattern> subfplist;

            // count frequency of current tree
            for (FPNode* leaf = hdr_table[baseItem]; leaf != NULL; leaf = leaf->next) {
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
            std::unordered_map<Item, FPNode*> tail_table;
            for (FPNode* leaf = hdr_table[baseItem]; leaf != NULL; leaf = leaf->next) {
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
                        curr = curr->child[item] = node;
                        if (tail_table.find(item) == tail_table.end()) {
                            fptree.hdr_table[item] = tail_table[item] = node;
                        } else {
                            tail_table[item] = tail_table[item]->next = node;
                        }
                    }
                    curr->cnt += leaf->cnt;
                }
            }

            base.emplace_back(baseItem);
            fplist.insert(base, item_freq[baseItem]);
            if (!fptree.empty()) {
                fptree.fpgrowth(base, min_sup, fplist);
            }
            base.pop_back();
        }
    }
}

bool FPTree::empty() {
    return root->child.size() == 0;
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
    if (input_file.is_open()) {
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
}

void output_patterns(const std::string& output_filename, const FPList& fplist, const size_t trxns_size) {
    std::ostringstream oss;
    fplist.output(oss, trxns_size);

    // Open output file and write the buffered output to it
    std::ofstream output_file(output_filename);
    if (output_file.is_open()) {
        auto output_str = oss.str();
        output_file.write(output_str.data(), output_str.size());
        output_file.close();
    }
}