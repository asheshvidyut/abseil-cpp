#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

#include "benchmark/benchmark.h"
#include "absl/container/btree_map.h"
#include "absl/container/internal/rbtree.h"

// Function to read words from the file
std::vector<std::string> ReadWords(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string word;
    if (file.is_open()) {
        while (file >> word) {
            words.push_back(word);
        }
        file.close();
    }
    return words;
}

static void BM_RBTree_Insert(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    for (auto _ : state) {
        absl::container_internal::RBTree<std::string, int> tree;
        int i = 0;
        for (const auto& word : words) {
            tree.insert(word, i++);
        }
    }
}
BENCHMARK(BM_RBTree_Insert);

static void BM_BTreeMap_Insert(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    for (auto _ : state) {
        absl::btree_map<std::string, int> map;
        int i = 0;
        for (const auto& word : words) {
            map[word] = i++;
        }
    }
}
BENCHMARK(BM_BTreeMap_Insert);

// Function to get a subset of words to search for
std::vector<std::string> GetSearchWords(const std::vector<std::string>& all_words, int count) {
    std::vector<std::string> search_words;
    if (all_words.empty() || count <= 0) return search_words;
    std::sample(all_words.begin(), all_words.end(), std::back_inserter(search_words),
                count, std::mt19937{std::random_device{}()});
    return search_words;
}

static void BM_RBTree_Search(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::container_internal::RBTree<std::string, int> tree;
    int i = 0;
    for (const auto& word : words) {
        tree.insert(word, i++);
    }
    auto search_words = GetSearchWords(words, words.size() / 10);

    for (auto _ : state) {
        for (const auto& word : search_words) {
            benchmark::DoNotOptimize(tree.search(word));
        }
    }
}
BENCHMARK(BM_RBTree_Search);

static void BM_BTreeMap_Search(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::btree_map<std::string, int> map;
    int i = 0;
    for (const auto& word : words) {
        map[word] = i++;
    }
    auto search_words = GetSearchWords(words, words.size() / 10);

    for (auto _ : state) {
        for (const auto& word : search_words) {
            benchmark::DoNotOptimize(map.find(word));
        }
    }
}
BENCHMARK(BM_BTreeMap_Search);

static void BM_RBTree_Iterate(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::container_internal::RBTree<std::string, int> tree;
    int i = 0;
    for (const auto& word : words) {
        tree.insert(word, i++);
    }

    for (auto _ : state) {
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            benchmark::DoNotOptimize(it.key());
            benchmark::DoNotOptimize(*it);
        }
    }
}
BENCHMARK(BM_RBTree_Iterate);

static void BM_BTreeMap_Iterate(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::btree_map<std::string, int> map;
    int i = 0;
    for (const auto& word : words) {
        map[word] = i++;
    }

    for (auto _ : state) {
        for (auto const& [key, val] : map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(val);
        }
    }
}
BENCHMARK(BM_BTreeMap_Iterate);

// BENCHMARK_MAIN() is in the BUILD file dependency
