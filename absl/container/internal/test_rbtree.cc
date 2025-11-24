#include "absl/container/internal/rbtree.h"
#include "gtest/gtest.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
#include <map> // Added for std::map in InsertAndTraverseWords test

namespace absl {
namespace container_internal {
namespace {

TEST(RBTreeTest, InsertAndTraverseWords) {
    RBTree<std::string, int> tree;
    std::vector<std::string> words;
    std::ifstream word_file("absl/container/internal/words.txt");
    ASSERT_TRUE(word_file.is_open());

    std::string word;
    int line_num = 0;
    while (std::getline(word_file, word)) {
        if (!word.empty()) {
            words.push_back(word);
            tree.insert(word, line_num++);
        }
    }
    word_file.close();

    std::vector<std::string> sorted_words = words;
    std::sort(sorted_words.begin(), sorted_words.end());

    std::vector<std::pair<std::string, int>> traversed_pairs;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        traversed_pairs.push_back({it.key(), *it});
    }

    // Create expected pairs for comparison
    std::map<std::string, int> expected_map;
    for (size_t i = 0; i < words.size(); ++i) {
        expected_map[words[i]] = i;
    }

    std::vector<std::pair<std::string, int>> expected_pairs;
    for (const auto& word : sorted_words) {
        expected_pairs.push_back({word, expected_map[word]});
    }

    EXPECT_EQ(traversed_pairs.size(), sorted_words.size());
    EXPECT_EQ(traversed_pairs, expected_pairs);

    // Verify search returns the correct line numbers
    for (size_t i = 0; i < words.size(); ++i) {
        auto* node = tree.search(words[i]);
        ASSERT_NE(node, nullptr) << "Word not found: " << words[i];
        EXPECT_EQ(node->value, expected_map[words[i]]) << "Value mismatch for word: " << words[i];
    }
}

TEST(RBTreeTest, OperatorSquareBrackets) {
    RBTree<std::string, int> tree;

    // Test insertion
    tree["apple"] = 10;
    EXPECT_EQ(tree.search("apple")->value, 10);

    // Test value update
    tree["apple"] = 20;
    EXPECT_EQ(tree.search("apple")->value, 20);

    // Test access
    EXPECT_EQ(tree["apple"], 20);

    // Test insertion of another element
    tree["banana"] = 30;
    EXPECT_EQ(tree.search("banana")->value, 30);
    EXPECT_EQ(tree["apple"], 20); // Verify apple is still there
}

TEST(RBTreeTest, Iterator) {
    RBTree<int, std::string> tree;
    tree.insert(10, "ten");
    tree.insert(5, "five");
    tree.insert(15, "fifteen");
    tree.insert(3, "three");
    tree.insert(7, "seven");

    std::vector<int> keys;
    std::vector<std::string> values;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        keys.push_back(it.key());
        values.push_back(*it);
    }
    std::vector<int> expected_keys = {3, 5, 7, 10, 15};
    std::vector<std::string> expected_values = {"three", "five", "seven", "ten", "fifteen"};
    EXPECT_EQ(keys, expected_keys);
    EXPECT_EQ(values, expected_values);

    auto it = tree.begin();
    EXPECT_EQ(it.key(), 3);
    EXPECT_EQ(*it, "three");
    ++it;
    EXPECT_EQ(it.key(), 5);
    it++;
    EXPECT_EQ(it.key(), 7);
    --it;
    EXPECT_EQ(it.key(), 5);
    it--;
    EXPECT_EQ(it.key(), 3);
    EXPECT_EQ(it == tree.begin(), true);

    int count = 0;
    for(it = tree.begin(); it != tree.end(); ++it) {
        count++;
    }
    EXPECT_EQ(count, 5);
}


TEST(RBTreeTest, ConstIterator) {
    RBTree<int, std::string> tree;
    tree.insert(10, "ten");
    tree.insert(5, "five");
    tree.insert(15, "fifteen");

    const RBTree<int, std::string>& const_tree = tree;

    std::vector<int> keys;
    for (auto it = const_tree.begin(); it != const_tree.end(); ++it) {
        keys.push_back(it.key());
    }
    std::vector<int> expected_keys = {5, 10, 15};
    EXPECT_EQ(keys, expected_keys);

    keys.clear();
    for (auto it = tree.cbegin(); it != tree.cend(); ++it) {
        keys.push_back(it.key());
    }
    EXPECT_EQ(keys, expected_keys);

    auto it = const_tree.begin();
    EXPECT_EQ(it.key(), 5);
    // *it = "new_five"; // This should not compile
}


TEST(RBTreeTest, LowerBound) {
    RBTree<int, std::string> tree;
    tree.insert(10, "ten");
    tree.insert(5, "five");
    tree.insert(15, "fifteen");
    tree.insert(3, "three");
    tree.insert(7, "seven");

    auto it = tree.lower_bound(7);
    EXPECT_EQ(it.key(), 7);
    EXPECT_EQ(*it, "seven");

    it = tree.lower_bound(6);
    EXPECT_EQ(it.key(), 7);
    EXPECT_EQ(*it, "seven");

    it = tree.lower_bound(15);
    EXPECT_EQ(it.key(), 15);
    EXPECT_EQ(*it, "fifteen");

    it = tree.lower_bound(16);
    EXPECT_EQ(it, tree.end());

    it = tree.lower_bound(1);
    EXPECT_EQ(it.key(), 3);
    EXPECT_EQ(*it, "three");

    RBTree<int, int> empty_tree;
    auto empty_it = empty_tree.lower_bound(5);
    EXPECT_EQ(empty_it, empty_tree.end());
}

TEST(RBTreeTest, Delete) {
    RBTree<int, std::string> tree;
    tree.insert(10, "ten");
    tree.insert(5, "five");
    tree.insert(15, "fifteen");
    tree.insert(3, "three");
    tree.insert(7, "seven");
    tree.insert(12, "twelve");
    tree.insert(18, "eighteen");

    EXPECT_TRUE(tree.deleteNode(7)); // Delete leaf
    EXPECT_EQ(tree.search(7), nullptr);
    EXPECT_TRUE(tree.deleteNode(15)); // Delete node with one child (left)
    EXPECT_EQ(tree.search(15), nullptr);
    EXPECT_TRUE(tree.deleteNode(5)); // Delete node with one child (right)
    EXPECT_EQ(tree.search(5), nullptr);
    EXPECT_TRUE(tree.deleteNode(10)); // Delete node with two children
    EXPECT_EQ(tree.search(10), nullptr);

    std::vector<int> keys;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        keys.push_back(it.key());
    }
    std::vector<int> expected_keys = {3, 12, 18};
    EXPECT_EQ(keys, expected_keys);

    EXPECT_FALSE(tree.deleteNode(100)); // Delete non-existent key

    EXPECT_TRUE(tree.deleteNode(12));
    EXPECT_TRUE(tree.deleteNode(3));
    EXPECT_TRUE(tree.deleteNode(18)); // Delete root
    EXPECT_EQ(tree.getRoot(), nullptr);

    RBTree<int, int> empty_tree;
    EXPECT_FALSE(empty_tree.deleteNode(1)); // Delete from empty
}

}  // namespace

TEST(RBTreeTest, RandomizedDelete) {
    RBTree<std::string, int> tree;
    std::vector<std::string> words;
    std::ifstream word_file("absl/container/internal/words.txt");
    ASSERT_TRUE(word_file.is_open());

    std::string word;
    int val = 0;
    int word_count = 0;
    while (std::getline(word_file, word) && word_count < 1000) {
        if (!word.empty()) {
            words.push_back(word);
            tree.insert(word, val++);
            word_count++;
        }
    }
    word_file.close();

    std::vector<std::string> shuffled_words = words;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffled_words.begin(), shuffled_words.end(), g);

    for (const auto& word_to_delete : shuffled_words) {
        EXPECT_TRUE(tree.deleteNode(word_to_delete)) << "Failed to delete: " << word_to_delete;

        std::string last_key = "";
        bool first = true;
        for (auto it = tree.begin(); it != tree.end(); ++it) {
            if (!first) {
                EXPECT_LT(last_key, it.key()) << "Tree not sorted after deleting: " << word_to_delete;
            }
            last_key = it.key();
            first = false;
        }
    }
    EXPECT_EQ(tree.getRoot(), nullptr);
}



TEST(RBTreeSubchildTest, SubchildCountAfterInsert) {
    RBTree<int, int> tree;
    tree.insert(10, 10);
    EXPECT_EQ(tree.getRoot()->subchild_count, 1);
    tree.insert(5, 5);
    EXPECT_EQ(tree.search(10)->subchild_count, 2);
    EXPECT_EQ(tree.search(5)->subchild_count, 1);
    tree.insert(15, 15);
    EXPECT_EQ(tree.search(10)->subchild_count, 3);
    EXPECT_EQ(tree.search(15)->subchild_count, 1);
    std::cout << "--- Tree after 3 inserts --- \n";
    tree.printTreeWithSubchildCount();
    tree.insert(3, 3);
    tree.insert(7, 7);
    EXPECT_EQ(tree.search(5)->subchild_count, 3);
    EXPECT_EQ(tree.search(10)->subchild_count, 5);
    std::cout << "--- Tree after all inserts --- \n";
    tree.printTreeWithSubchildCount();
}

TEST(RBTreeSubchildTest, SubchildCountAfterDelete) {
    RBTree<int, int> tree;
    for (int i = 1; i <= 7; ++i) tree.insert(i, i);
    EXPECT_EQ(tree.getRoot()->subchild_count, 7);
    std::cout << "--- Tree before delete --- \n";
    tree.printTreeWithSubchildCount();

    tree.deleteNode(3);
    EXPECT_EQ(tree.getRoot()->subchild_count, 6);
    EXPECT_EQ(tree.search(1)->subchild_count, 1);
    std::cout << "--- Tree after delete 3 --- \n";
    tree.printTreeWithSubchildCount();

    tree.deleteNode(6);
    EXPECT_EQ(tree.getRoot()->subchild_count, 5);
    tree.deleteNode(1);
    EXPECT_EQ(tree.getRoot()->subchild_count, 4);
    tree.deleteNode(4); // Root deletion
    EXPECT_EQ(tree.getRoot()->subchild_count, 3);
    std::cout << "--- Tree after all deletes --- \n";
    tree.printTreeWithSubchildCount();
}

TEST(RBTreeSubchildTest, FindKthLargestBasic) {
    RBTree<int, int> tree;
    std::vector<int> nums = {10, 5, 15, 3, 7, 12, 18};
    for (int n : nums) tree.insert(n, n);
    // Sorted: 3, 5, 7, 10, 12, 15, 18

    EXPECT_EQ(tree.findKthLargest(1)->key, 18);
    EXPECT_EQ(tree.findKthLargest(2)->key, 15);
    EXPECT_EQ(tree.findKthLargest(3)->key, 12);
    EXPECT_EQ(tree.findKthLargest(4)->key, 10);
    EXPECT_EQ(tree.findKthLargest(5)->key, 7);
    EXPECT_EQ(tree.findKthLargest(6)->key, 5);
    EXPECT_EQ(tree.findKthLargest(7)->key, 3);
}

TEST(RBTreeSubchildTest, FindKthLargestEdgeCases) {
    RBTree<int, int> tree;
    EXPECT_EQ(tree.findKthLargest(1), nullptr);
    tree.insert(10, 10);
    EXPECT_EQ(tree.findKthLargest(1)->key, 10);
    EXPECT_EQ(tree.findKthLargest(0), nullptr);
    EXPECT_EQ(tree.findKthLargest(2), nullptr);

    tree.insert(5, 5);
    tree.insert(15, 15);
    // 5, 10, 15
    EXPECT_EQ(tree.findKthLargest(1)->key, 15);
    EXPECT_EQ(tree.findKthLargest(3)->key, 5);
    EXPECT_EQ(tree.findKthLargest(4), nullptr);
}



TEST(RBTreeSubchildTest, FindKthLargestWords) {
    RBTree<std::string, int> tree;
    std::vector<std::string> words;
    std::set<std::string> unique_words;
    std::ifstream word_file("absl/container/internal/words.txt");
    ASSERT_TRUE(word_file.is_open());

    std::string word;
    int val = 0;
    while (std::getline(word_file, word)) {
        if (!word.empty()) {
            if (unique_words.find(word) == unique_words.end()) {
                unique_words.insert(word);
                tree.insert(word, val++);
            }
        }
    }
    word_file.close();

    for(const auto& w : unique_words) {
        words.push_back(w);
    }
    std::sort(words.begin(), words.end());

    int n = words.size();
    for (int k = 1; k <= n; ++k) {
        Node<std::string, int>* node = tree.findKthLargest(k);
        ASSERT_NE(node, nullptr) << "Failed for k = " << k;
        EXPECT_EQ(node->key, words[n - k]) << "Mismatch for k = " << k;
    }
}

}  // namespace container_internal
}  // namespace absl