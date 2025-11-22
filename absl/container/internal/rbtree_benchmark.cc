#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <map>

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

// Function to generate a vector of random UUIDs
std::vector<std::string> GenerateUUIDs(size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::vector<std::string> uuids;
    uuids.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        std::stringstream ss;
        ss << std::hex;
        for (int j = 0; j < 8; j++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int j = 0; j < 4; j++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (int j = 0; j < 3; j++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (int j = 0; j < 3; j++) {
            ss << dis(gen);
        }
        ss << "-";
        for (int j = 0; j < 12; j++) {
            ss << dis(gen);
        }
        uuids.push_back(ss.str());
    }
    return uuids;
}

static void BM_BTreeMap_Insert(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    for (auto _ : state) {
        absl::btree_map<std::string, int> map;
        int i = 0;
        for (const auto& word : words) {
            map[word] = i++;
        }
    }
    state.counters["words"] = benchmark::Counter(words.size());
}
BENCHMARK(BM_BTreeMap_Insert);

static void BM_StdMap_Insert(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    for (auto _ : state) {
        std::map<std::string, int> map;
        int i = 0;
        for (const auto& word : words) {
            map[word] = i++;
        }
    }
    state.counters["words"] = benchmark::Counter(words.size());
}
BENCHMARK(BM_StdMap_Insert);

static void BM_RBTree_Insert(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    for (auto _ : state) {
        absl::container_internal::RBTree<std::string, int> tree;
        int i = 0;
        for (const auto& word : words) {
            tree.insert(word, i++);
        }
    }
    state.counters["words"] = benchmark::Counter(words.size());
}
BENCHMARK(BM_RBTree_Insert);

// Function to get a subset of words to search for
std::vector<std::string> GetSearchWords(const std::vector<std::string>& all_words, int count) {
    std::vector<std::string> search_words;
    if (all_words.empty() || count <= 0) return search_words;
    std::sample(all_words.begin(), all_words.end(), std::back_inserter(search_words),
                count, std::mt19937{std::random_device{}()});
    return search_words;
}

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
    state.counters["total_words"] = benchmark::Counter(words.size());
    state.counters["search_words"] = benchmark::Counter(search_words.size());
}
BENCHMARK(BM_BTreeMap_Search);

static void BM_StdMap_Search(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    std::map<std::string, int> map;
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
    state.counters["total_words"] = benchmark::Counter(words.size());
    state.counters["search_words"] = benchmark::Counter(search_words.size());
}
BENCHMARK(BM_StdMap_Search);

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
    state.counters["total_words"] = benchmark::Counter(words.size());
    state.counters["search_words"] = benchmark::Counter(search_words.size());
}
BENCHMARK(BM_RBTree_Search);

static void BM_BTreeMap_Iterate_All(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::btree_map<std::string, int> map;
    int i = 0;
    for (const auto& word : words) {
        map[word] = i++;
    }

    size_t iterated_count = 0;
    for (auto _ : state) {
        iterated_count = 0;
        for (auto const& [key, val] : map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(val);
            iterated_count++;
        }
    }
    state.counters["words_iterated"] = benchmark::Counter(iterated_count);
    state.counters["total_words"] = benchmark::Counter(words.size());
}
BENCHMARK(BM_BTreeMap_Iterate_All);

static void BM_StdMap_Iterate_All(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    std::map<std::string, int> map;
    int i = 0;
    for (const auto& word : words) {
        map[word] = i++;
    }

    size_t iterated_count = 0;
    for (auto _ : state) {
        iterated_count = 0;
        for (auto const& [key, val] : map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(val);
            iterated_count++;
        }
    }
    state.counters["words_iterated"] = benchmark::Counter(iterated_count);
    state.counters["total_words"] = benchmark::Counter(words.size());
}
BENCHMARK(BM_StdMap_Iterate_All);

static void BM_RBTree_Iterate_All(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::container_internal::RBTree<std::string, int> tree;
    int i = 0;
    for (const auto& word : words) {
        tree.insert(word, i++);
    }

    size_t iterated_count = 0;
    for (auto _ : state) {
        iterated_count = 0;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            benchmark::DoNotOptimize(it.key());
            benchmark::DoNotOptimize(*it);
            iterated_count++;
        }
    }
    state.counters["words_iterated"] = benchmark::Counter(iterated_count);
    state.counters["total_words"] = benchmark::Counter(words.size());
}
BENCHMARK(BM_RBTree_Iterate_All);

// Memory benchmarks
static void BM_BTreeMap_Memory(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::btree_map<std::string, int> map;
    int i = 0;
    for (const auto& word : words) {
        map[word] = i++;
    }
    
    // BTree stores multiple values per node (256 bytes per node typically)
    // Calculate based on actual structure
    size_t element_count = map.size();
    size_t total_key_size = 0;
    for (const auto& [key, val] : map) {
        total_key_size += key.size();
    }
    
    // BTree has better memory efficiency
    // Estimate based on: node overhead + packed values
    // Each node is ~256 bytes and holds ~20-50 values
    size_t estimated_nodes = (element_count + 20) / 20; // Rough estimate
    size_t estimated_memory = estimated_nodes * 256 +  // Node overhead
                              total_key_size +         // Key strings
                              element_count * sizeof(int) +  // Values
                              element_count * (sizeof(std::string) - sizeof(char*)); // String object overhead
    
    for (auto _ : state) {
        state.SetBytesProcessed(estimated_memory);
        benchmark::DoNotOptimize(map);
    }
    state.counters["elements"] = benchmark::Counter(element_count);
    state.counters["bytes_per_element"] = benchmark::Counter(estimated_memory / static_cast<double>(element_count));
    state.counters["total_bytes"] = benchmark::Counter(estimated_memory);
    state.counters["estimated_nodes"] = benchmark::Counter(estimated_nodes);
}
BENCHMARK(BM_BTreeMap_Memory);

static void BM_StdMap_Memory(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    std::map<std::string, int> map;
    int i = 0;
    for (const auto& word : words) {
        map[word] = i++;
    }
    
    // std::map typically uses red-black tree internally
    // Each node has: 3 pointers (parent, left, right) = 24 bytes
    //                Color = 1 byte (usually stored in pointer LSB)
    //                Key (string) + Value (int) = variable
    //                Padding = ~7 bytes
    // Total per node overhead: ~32 bytes + key size + value size
    size_t element_count = map.size();
    size_t total_key_size = 0;
    for (const auto& [key, val] : map) {
        total_key_size += key.size();
    }
    
    // Estimate based on red-black tree structure
    size_t estimated_memory = element_count * 32 +  // Node overhead
                              total_key_size +     // Key strings
                              element_count * sizeof(int) +  // Values
                              element_count * (sizeof(std::string) - sizeof(char*)); // String object overhead
    
    for (auto _ : state) {
        state.SetBytesProcessed(estimated_memory);
        benchmark::DoNotOptimize(map);
    }
    state.counters["elements"] = benchmark::Counter(element_count);
    state.counters["bytes_per_element"] = benchmark::Counter(estimated_memory / static_cast<double>(element_count));
    state.counters["total_bytes"] = benchmark::Counter(estimated_memory);
}
BENCHMARK(BM_StdMap_Memory);

static void BM_RBTree_Memory(benchmark::State& state) {
    auto words = ReadWords("absl/container/internal/words.txt");
    absl::container_internal::RBTree<std::string, int> tree;
    int i = 0;
    for (const auto& word : words) {
        tree.insert(word, i++);
    }
    
    // Estimate memory usage for RBTree
    // Each node has: 3 pointers (parent, left, right) = 24 bytes
    //                2 pointers (prev, next) = 16 bytes
    //                2 pointers (min_node, max_node) = 16 bytes
    //                Color = 1 byte
    //                Key (string) + Value (int) = variable
    //                Padding = ~7 bytes
    // Total per node overhead: ~64 bytes + key size + value size
    size_t node_count = 0;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        node_count++;
    }
    size_t total_key_size = 0;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        total_key_size += it.key().size();
    }
    
    // Estimate: node overhead + key storage + value storage
    size_t estimated_memory = node_count * 64 +  // Node overhead
                              total_key_size +   // Key strings
                              node_count * sizeof(int) +  // Values
                              node_count * (sizeof(std::string) - sizeof(char*)); // String object overhead
    
    for (auto _ : state) {
        state.SetBytesProcessed(estimated_memory);
        benchmark::DoNotOptimize(tree);
    }
    state.counters["nodes"] = benchmark::Counter(node_count);
    state.counters["bytes_per_node"] = benchmark::Counter(estimated_memory / static_cast<double>(node_count));
    state.counters["total_bytes"] = benchmark::Counter(estimated_memory);
}
BENCHMARK(BM_RBTree_Memory);

// Parameterized insert benchmarks with different sizes
static void BM_BTreeMap_Insert_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    for (auto _ : state) {
        absl::btree_map<std::string, int> map;
        int i = 0;
        for (const auto& uuid : uuids) {
            map[uuid] = i++;
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
}
BENCHMARK(BM_BTreeMap_Insert_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

static void BM_StdMap_Insert_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    for (auto _ : state) {
        std::map<std::string, int> map;
        int i = 0;
        for (const auto& uuid : uuids) {
            map[uuid] = i++;
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
}
BENCHMARK(BM_StdMap_Insert_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

static void BM_RBTree_Insert_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    for (auto _ : state) {
        absl::container_internal::RBTree<std::string, int> tree;
        int i = 0;
        for (const auto& uuid : uuids) {
            tree.insert(uuid, i++);
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
}
BENCHMARK(BM_RBTree_Insert_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

// Parameterized iterate benchmarks with different sizes
static void BM_BTreeMap_Iterate_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    absl::btree_map<std::string, int> map;
    int i = 0;
    for (const auto& uuid : uuids) {
        map[uuid] = i++;
    }
    
    size_t iterated_count = 0;
    for (auto _ : state) {
        iterated_count = 0;
        for (auto const& [key, val] : map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(val);
            iterated_count++;
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
    state.counters["iterated"] = benchmark::Counter(iterated_count);
}
BENCHMARK(BM_BTreeMap_Iterate_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

static void BM_StdMap_Iterate_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    std::map<std::string, int> map;
    int i = 0;
    for (const auto& uuid : uuids) {
        map[uuid] = i++;
    }
    
    size_t iterated_count = 0;
    for (auto _ : state) {
        iterated_count = 0;
        for (auto const& [key, val] : map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(val);
            iterated_count++;
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
    state.counters["iterated"] = benchmark::Counter(iterated_count);
}
BENCHMARK(BM_StdMap_Iterate_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

static void BM_RBTree_Iterate_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    absl::container_internal::RBTree<std::string, int> tree;
    int i = 0;
    for (const auto& uuid : uuids) {
        tree.insert(uuid, i++);
    }
    
    size_t iterated_count = 0;
    for (auto _ : state) {
        iterated_count = 0;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            benchmark::DoNotOptimize(it.key());
            benchmark::DoNotOptimize(*it);
            iterated_count++;
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
    state.counters["iterated"] = benchmark::Counter(iterated_count);
}
BENCHMARK(BM_RBTree_Iterate_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

// Parameterized search benchmarks with different sizes
static void BM_BTreeMap_Search_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    absl::btree_map<std::string, int> map;
    int i = 0;
    for (const auto& uuid : uuids) {
        map[uuid] = i++;
    }
    
    // Search for 10% of the elements (or at least 10, or all if less than 10)
    size_t search_count = std::max(size_t(10), std::min(num_elements / 10, num_elements));
    std::vector<std::string> search_uuids;
    search_uuids.reserve(search_count);
    std::sample(uuids.begin(), uuids.end(), std::back_inserter(search_uuids),
                search_count, std::mt19937{std::random_device{}()});
    
    for (auto _ : state) {
        for (const auto& uuid : search_uuids) {
            benchmark::DoNotOptimize(map.find(uuid));
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
    state.counters["searches"] = benchmark::Counter(search_count);
}
BENCHMARK(BM_BTreeMap_Search_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

static void BM_StdMap_Search_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    std::map<std::string, int> map;
    int i = 0;
    for (const auto& uuid : uuids) {
        map[uuid] = i++;
    }
    
    // Search for 10% of the elements (or at least 10, or all if less than 10)
    size_t search_count = std::max(size_t(10), std::min(num_elements / 10, num_elements));
    std::vector<std::string> search_uuids;
    search_uuids.reserve(search_count);
    std::sample(uuids.begin(), uuids.end(), std::back_inserter(search_uuids),
                search_count, std::mt19937{std::random_device{}()});
    
    for (auto _ : state) {
        for (const auto& uuid : search_uuids) {
            benchmark::DoNotOptimize(map.find(uuid));
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
    state.counters["searches"] = benchmark::Counter(search_count);
}
BENCHMARK(BM_StdMap_Search_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

static void BM_RBTree_Search_Size(benchmark::State& state) {
    size_t num_elements = state.range(0);
    auto uuids = GenerateUUIDs(num_elements);
    
    absl::container_internal::RBTree<std::string, int> tree;
    int i = 0;
    for (const auto& uuid : uuids) {
        tree.insert(uuid, i++);
    }
    
    // Search for 10% of the elements (or at least 10, or all if less than 10)
    size_t search_count = std::max(size_t(10), std::min(num_elements / 10, num_elements));
    std::vector<std::string> search_uuids;
    search_uuids.reserve(search_count);
    std::sample(uuids.begin(), uuids.end(), std::back_inserter(search_uuids),
                search_count, std::mt19937{std::random_device{}()});
    
    for (auto _ : state) {
        for (const auto& uuid : search_uuids) {
            benchmark::DoNotOptimize(tree.search(uuid));
        }
    }
    state.counters["elements"] = benchmark::Counter(num_elements);
    state.counters["searches"] = benchmark::Counter(search_count);
}
BENCHMARK(BM_RBTree_Search_Size)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(100000);

// BENCHMARK_MAIN() is in the BUILD file dependency
