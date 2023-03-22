#include <omp.h>
#include <pthread.h>

#include <array>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

class FP {
   protected:
    std::vector<std::vector<int>> transactions;
    std::array<int, 1000> item_count;
    std::vector<std::pair<std::vector<int>, float>> freq_patterns;

   public:
    FP() {
        item_count.fill(0);
    }
    ~FP() {}

    void input(char* filename);
    void output(char* filename);
    virtual void process(double min_support) = 0;
};

class FPGrowth : public FP {
   protected:
   public:
    FPGrowth() : FP() {}
    ~FPGrowth(){};
    void process(double min_support) override;
};

int main(int argc, char** argv) {
    assert(argc == 4);
    double min_support = atof(argv[1]);
    char* input_filename = argv[2];
    char* output_filename = argv[3];

    FPGrowth dataProcessor;
    dataProcessor.input(input_filename);
    dataProcessor.process(min_support);
    dataProcessor.output(output_filename);
}

void FP::input(char* filename) {
    std::ifstream file;
    std::string linebuf;
    file.open(filename);

    while (getline(file, linebuf)) {
        std::vector<int> databuf;
        size_t start = 0;
        size_t end = linebuf.find_first_of(',');

        while (true) {
            int item = std::stoi(linebuf.substr(start, end - start));
            databuf.emplace_back(item);
            item_count[item]++;
            if (end == std::string::npos)
                break;
            start = end + 1;
            end = linebuf.find_first_of(',', start);
        }
        transactions.emplace_back(databuf);
    }

    // Debug
    // for (int i = 0; i < transactions.size(); i++) {
    //     for (int j = 0; j < transactions[i].size(); j++) {
    //         std::cout << transactions[i][j] << " ";
    //     }
    //     std::cout << "\n";
    // }
    // for (int i = 0; i < 1000; i++)
    //     std::cout << item_count[i] << " ";
    // std::cout << "\n";

    file.close();
}

void FP::output(char* filename) {
    std::ofstream file;
    file.open(filename);

    for (int i = 0; i < (int)freq_patterns.size(); i++) {
        const auto& transaction = freq_patterns[i].first;
        const auto& support = freq_patterns[i].second;

        file << transaction[0];
        for (int j = 1; j < (int)transaction.size(); j++)
            file << ',' << transaction[j];
        file << std::fixed
             << std::setprecision(4)
             << ':' << support << '\n';
    }

    file.close();
}

void FPGrowth::process(double min_support) {
    // Debug
    // for (int i = 0; i < (int)transactions.size(); i++)
    //     freq_patterns.emplace_back(transactions[i], min_support / (i + 1));
}
