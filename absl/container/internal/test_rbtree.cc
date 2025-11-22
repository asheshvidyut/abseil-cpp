#include "absl/container/internal/rbtree.h"
#include "gtest/gtest.h"
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

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

    std::vector<std::string> traversed_words;
    for (auto it = tree.begin(); it != tree.end(); ++it) {
        traversed_words.push_back(it.key());
    }

    EXPECT_EQ(traversed_words.size(), sorted_words.size());
    EXPECT_EQ(traversed_words, sorted_words);
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

}  // namespace container_internal
}  // namespace absl

}