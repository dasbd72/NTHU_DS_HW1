#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

struct FrequentPatterns {
    struct Node {
        double freq;
        std::unordered_map<int, Node*> child;
        Node() : freq(-1) {}
        Node(double freq) : freq(freq) {}
        // return if current node is an end of frequent pattern
        bool isfp() const {
            return freq != -1;
        }
    };

    Node* root;
    size_t _size;

    FrequentPatterns() {
        root = new Node();
    }
    ~FrequentPatterns() {
        deleteTree(root);
    }

    void insert(const std::vector<int>& lst, double freq) {
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
        if (curr->isfp()) {
            cerr << "Err: Repeated frequent patterns\n";
        }

        curr->freq = freq;
        _size++;
    }

    double pop(const std::vector<int>& lst) {
        double retFreq = -1;
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
        if (!curr->isfp()) {
            cerr << "Err: Frequent pattern not exist\n";
        }
        retFreq = curr->freq;
        curr->freq = -1;
        _size--;
        return retFreq;
    }

    size_t size() const {
        return _size;
    }

    void deleteTree(Node* node) {
        for (auto& pair : node->child) {
            deleteTree(pair.second);
        }
        delete node;
    }
};

FrequentPatterns parse(string filename) {
    FrequentPatterns fps;
    ifstream file(filename);
    if (file.is_open()) {
        string str;
        while (getline(file, str)) {
            vector<int> pt;
            pt.reserve(200);
            double freq;

            size_t sep = str.find(':');
            if (sep == string::npos) {
                cerr << "Err: Wrong format.\n";
            }

            freq = stof(str.substr(sep + 1, string::npos));

            str = str.substr(0, sep);
            size_t start = 0;
            size_t end = str.find(',');

            while (true) {
                int item = stoi(str.substr(start, end - start));
                pt.emplace_back(item);
                if (end == string::npos)
                    break;
                start = end + 1;
                end = str.find(',', start);
            }
            sort(pt.begin(), pt.end());
            fps.insert(pt, freq);
        }
    } else {
        cerr << "Err: File not opened.\n";
    }
    cout << "Parsed " << filename << "\n";
    return fps;
}

void deparse(string filename, FrequentPatterns& fps) {
    ifstream file(filename);
    if (file.is_open()) {
        string str;
        while (getline(file, str)) {
            vector<int> pt;
            pt.reserve(200);
            double freq;

            size_t sep = str.find(':');
            assert(sep != string::npos);

            freq = stof(str.substr(sep + 1, string::npos));

            str = str.substr(0, sep);
            size_t start = 0;
            size_t end = str.find(',');

            while (true) {
                int item = stoi(str.substr(start, end - start));
                pt.emplace_back(item);
                if (end == string::npos)
                    break;
                start = end + 1;
                end = str.find(',', start);
            }
            sort(pt.begin(), pt.end());

            if (fps.pop(pt) != freq) {
                cerr << "Err: Freqeuncy not matched.\n";
            }
        }
    } else {
        cerr << "Err: File not opened.\n";
    }
    cout << "Deparsed " << filename << "\n";
}

int main(int argc, char** argv) {
    assert(argc == 3);
    string file1 = argv[1];
    string file2 = argv[2];

    FrequentPatterns fps = parse(file1);
    deparse(file2, fps);

    if (fps.size() != 0) {
        cerr << "Err: Not equal number of lines.\n";
    }
    cout << "Passed\n";
}
