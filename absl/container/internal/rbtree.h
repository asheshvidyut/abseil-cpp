#ifndef ABSL_CONTAINER_INTERNAL_RBTREE_H_
#define ABSL_CONTAINER_INTERNAL_RBTREE_H_

#include <iostream>
#include <queue>

namespace absl {
namespace container_internal {

enum class Color { RED, BLACK };

template <typename Key, typename Value>
struct Node {
    Key key;
    Value value;
    Color color;
    Node *parent, *left, *right;
    Node *prev, *next; // Doubly linked list pointers
    Node *min_node, *max_node; // Min and Max nodes in the subtree rooted here

    Node(Key k, Value v) : key(k), value(v), color(Color::RED), parent(nullptr), left(nullptr), right(nullptr), prev(nullptr), next(nullptr), min_node(this), max_node(this) {}
};

template <typename Key, typename Value>
class RBTree {
public:
    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = Value*;
        using reference = Value&;

        iterator(Node<Key, Value>* node = nullptr) : current_node(node) {}

        reference operator*() const { return current_node->value; }
        pointer operator->() const { return &current_node->value; }
        const Key& key() const { return current_node->key; }

        // Prefix increment
        iterator& operator++() {
            if (current_node) current_node = current_node->next;
            return *this;
        }

        // Postfix increment
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        // Prefix decrement
        iterator& operator--() {
            if (current_node) current_node = current_node->prev;
            // TODO: Handle decrementing from end()
            return *this;
        }

        // Postfix decrement
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return current_node == other.current_node;
        }

        bool operator!=(const iterator& other) const {
            return current_node != other.current_node;
        }

    private:
        Node<Key, Value>* current_node;
        friend class RBTree;
    };

    RBTree() : root(nullptr) {}
    ~RBTree();

    void insert(const Key& key, const Value& value);
    bool deleteNode(const Key& key);
    Node<Key, Value>* search(const Key& key);
    void printTree();
    Node<Key, Value>* getRoot() const { return root; }
    iterator begin() { return iterator(root ? root->min_node : nullptr); }
    iterator end() { return iterator(nullptr); }
    Value& operator[](const Key& key);

    iterator lower_bound(const Key& key) {
        Node<Key, Value>* current = root;
        Node<Key, Value>* result = nullptr;
        while (current != nullptr) {
            if (!(current->key < key)) { // current->key >= key
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return iterator(result);
    }

    class const_iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const Value;
        using difference_type = std::ptrdiff_t;
        using pointer = const Value*;
        using reference = const Value&;

        const_iterator(const Node<Key, Value>* node = nullptr) : current_node(node) {}
        const_iterator(const iterator& it) : current_node(it.current_node) {}

        reference operator*() const { return current_node->value; }
        pointer operator->() const { return &current_node->value; }
        const Key& key() const { return current_node->key; }

        const_iterator& operator++() { if (current_node) current_node = current_node->next; return *this; }
        const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }
        const_iterator& operator--() { if (current_node) current_node = current_node->prev; return *this; }
        const_iterator operator--(int) { const_iterator tmp = *this; --(*this); return tmp; }

        bool operator==(const const_iterator& other) const { return current_node == other.current_node; }
        bool operator!=(const const_iterator& other) const { return current_node != other.current_node; }

    private:
        const Node<Key, Value>* current_node;
        friend class RBTree;
    };

    const_iterator begin() const { return const_iterator(root ? root->min_node : nullptr); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

private:
    Node<Key, Value>* root;

    // Helper functions
    void leftRotate(Node<Key, Value>* x);
    void rightRotate(Node<Key, Value>* y);
    void fixInsertViolation(Node<Key, Value>* z);
    void fixDeleteViolation(Node<Key, Value>* x, Node<Key, Value>* parent);
    Node<Key, Value>* minimum(Node<Key, Value>* node);
    Node<Key, Value>* maximum(Node<Key, Value>* node);
    void transplant(Node<Key, Value>* u, Node<Key, Value>* v);
    void printTreeHelper(Node<Key, Value>* node, std::string indent, bool last);
    void updateMinMax(Node<Key, Value>* node);
};

template <typename Key, typename Value>
void RBTree<Key, Value>::leftRotate(Node<Key, Value>* x) {
    Node<Key, Value>* y = x->right;
    x->right = y->left;
    if (y->left != nullptr) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
        this->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;

    updateMinMax(x);
    updateMinMax(y);
}

template <typename Key, typename Value>
void RBTree<Key, Value>::rightRotate(Node<Key, Value>* x) {
    Node<Key, Value>* y = x->left;
    x->left = y->right;
    if (y->right != nullptr) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
        this->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;

    updateMinMax(x);
    updateMinMax(y);
}

template <typename Key, typename Value>
Node<Key, Value>* RBTree<Key, Value>::minimum(Node<Key, Value>* node) {
    return node ? node->min_node : nullptr;
}

template <typename Key, typename Value>
Node<Key, Value>* RBTree<Key, Value>::maximum(Node<Key, Value>* node) {
    return node ? node->max_node : nullptr;
}

template <typename Key, typename Value>
void RBTree<Key, Value>::updateMinMax(Node<Key, Value>* node) {
    if (!node) return;
    node->min_node = node;
    node->max_node = node;
    if (node->left != nullptr) {
        node->min_node = node->left->min_node;
    }
    if (node->right != nullptr) {
        node->max_node = node->right->max_node;
    }
}

template <typename Key, typename Value>
void RBTree<Key, Value>::transplant(Node<Key, Value>* u, Node<Key, Value>* v) {
    if (u->parent == nullptr) {
        root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    if (v != nullptr) {
        v->parent = u->parent;
    }
}

template <typename Key, typename Value>
void RBTree<Key, Value>::insert(const Key& key, const Value& value) {
    Node<Key, Value>* node = new Node<Key, Value>(key, value);
    Node<Key, Value>* y = nullptr;
    Node<Key, Value>* x = this->root;

    while (x != nullptr) {
        y = x;
        if (node->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    node->parent = y;
    if (y == nullptr) {
        root = node;
    } else if (node->key < y->key) {
        y->left = node;
    } else {
        y->right = node;
    }

    if (node->parent == nullptr) {
        node->color = Color::BLACK;
        return;
    }
    // Update min/max pointers up the tree
    Node<Key, Value>* temp = node;
    while (temp != nullptr) {
        updateMinMax(temp);
        temp = temp->parent;
    }

    // Link into doubly linked list
    Node<Key, Value>* pred = nullptr;
    if (node->left != nullptr) {
        pred = node->left->max_node;
    } else {
        temp = node;
        while (temp->parent != nullptr && temp == temp->parent->left) {
            temp = temp->parent;
        }
        pred = temp->parent;
    }

    Node<Key, Value>* succ = nullptr;
    if (node->right != nullptr) {
        succ = node->right->min_node;
    } else {
        temp = node;
        while (temp->parent != nullptr && temp == temp->parent->right) {
            temp = temp->parent;
        }
        succ = temp->parent;
    }

    node->prev = pred;
    node->next = succ;
    if (pred != nullptr) pred->next = node;
    if (succ != nullptr) succ->prev = node;

    if (node->parent == nullptr) {
        node->color = Color::BLACK;
        return;
    }
    if (node->parent->parent == nullptr) {
        return;
    }

    fixInsertViolation(node);
}

template <typename Key, typename Value>
void RBTree<Key, Value>::fixInsertViolation(Node<Key, Value>* k) {
    Node<Key, Value>* u;
    while (k->parent != nullptr && k->parent->color == Color::RED) {
        if (k->parent == k->parent->parent->right) {
            u = k->parent->parent->left;
            if (u != nullptr && u->color == Color::RED) {
                u->color = Color::BLACK;
                k->parent->color = Color::BLACK;
                k->parent->parent->color = Color::RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->left) {
                    k = k->parent;
                    rightRotate(k);
                }
                k->parent->color = Color::BLACK;
                k->parent->parent->color = Color::RED;
                leftRotate(k->parent->parent);
            }
        } else {
            u = k->parent->parent->right;
            if (u != nullptr && u->color == Color::RED) {
                u->color = Color::BLACK;
                k->parent->color = Color::BLACK;
                k->parent->parent->color = Color::RED;
                k = k->parent->parent;
            } else {
                if (k == k->parent->right) {
                    k = k->parent;
                    leftRotate(k);
                }
                k->parent->color = Color::BLACK;
                k->parent->parent->color = Color::RED;
                rightRotate(k->parent->parent);
            }
        }
        if (k == root) {
            break;
        }
    }
    root->color = Color::BLACK;
}

template <typename Key, typename Value>
Node<Key, Value>* RBTree<Key, Value>::search(const Key& key) {
    Node<Key, Value>* current = root;
    while (current != nullptr) {
        if (key == current->key) {
            return current;
        } else if (key < current->key) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    return nullptr;  // Not found
}

template <typename Key, typename Value>
bool RBTree<Key, Value>::deleteNode(const Key& key) {
    Node<Key, Value>* z = search(key);
    if (z == nullptr) return false;

    // Unlink from doubly linked list
    if (z->prev != nullptr) z->prev->next = z->next;
    if (z->next != nullptr) z->next->prev = z->prev;

    Node<Key, Value>* y = z;
    Color y_original_color = y->color;
    Node<Key, Value>* x;
    Node<Key, Value>* x_parent;

    if (z->left == nullptr) {
        x = z->right;
        x_parent = z->parent; // x_parent is z->parent
        transplant(z, x);
    } else if (z->right == nullptr) {
        x = z->left;
        x_parent = z->parent; // x_parent is z->parent
        transplant(z, x);
    } else {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        x_parent = y; // x_parent is y if x is y's child, otherwise it's y->parent

        if (y->parent == z) {
            x_parent = y;
        } else {
            x_parent = y->parent;
            transplant(y, x); // x replaces y
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(z, y); // y replaces z
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    delete z;

    // Update min/max on ancestors of where y was originally. 
    Node<Key, Value>* curr = x_parent;
    while(curr != nullptr) { 
        updateMinMax(curr); 
        curr = curr->parent; 
    }

    if (y_original_color == Color::BLACK) {
        fixDeleteViolation(x, x_parent);
    }

    return true;
}

template <typename Key, typename Value>
void RBTree<Key, Value>::fixDeleteViolation(Node<Key, Value>* x, Node<Key, Value>* parent) {
    while (x != root && (x == nullptr || x->color == Color::BLACK)) {
        if (x == parent->left) {
            Node<Key, Value>* w = parent->right; // Sibling
            if (w->color == Color::RED) { // Case 1
                w->color = Color::BLACK;
                parent->color = Color::RED;
                leftRotate(parent);
                w = parent->right;
            }
            if ((w->left == nullptr || w->left->color == Color::BLACK) &&
                (w->right == nullptr || w->right->color == Color::BLACK)) { // Case 2
                w->color = Color::RED;
                x = parent;
                parent = x->parent;
            } else {
                if (w->right == nullptr || w->right->color == Color::BLACK) { // Case 3
                    if (w->left != nullptr) w->left->color = Color::BLACK;
                    w->color = Color::RED;
                    rightRotate(w);
                    w = parent->right;
                }
                // Case 4
                w->color = parent->color;
                parent->color = Color::BLACK;
                if (w->right != nullptr) w->right->color = Color::BLACK;
                leftRotate(parent);
                x = root; // Terminate
            }
        } else { // x == parent->right
            Node<Key, Value>* w = parent->left; // Sibling
            if (w->color == Color::RED) { // Case 1
                w->color = Color::BLACK;
                parent->color = Color::RED;
                rightRotate(parent);
                w = parent->left;
            }
            if ((w->right == nullptr || w->right->color == Color::BLACK) &&
                (w->left == nullptr || w->left->color == Color::BLACK)) { // Case 2
                w->color = Color::RED;
                x = parent;
                parent = x->parent;
            } else {
                if (w->left == nullptr || w->left->color == Color::BLACK) { // Case 3
                    if (w->right != nullptr) w->right->color = Color::BLACK;
                    w->color = Color::RED;
                    leftRotate(w);
                    w = parent->left;
                }
                // Case 4
                w->color = parent->color;
                parent->color = Color::BLACK;
                if (w->left != nullptr) w->left->color = Color::BLACK;
                rightRotate(parent);
                x = root; // Terminate
            }
        }
    }
    if (x != nullptr) x->color = Color::BLACK;
}

template <typename Key, typename Value>
void RBTree<Key, Value>::printTree() {
    if (root) {
        printTreeHelper(this->root, "", true);
    }
}

template <typename Key, typename Value>
void RBTree<Key, Value>::printTreeHelper(Node<Key, Value>* node, std::string indent, bool last) {
    if (node != nullptr) {
        std::cout << indent;
        if (last) {
            std::cout << "R----";
            indent += "     ";
        } else {
            std::cout << "L----";
            indent += "|    ";
        }

        std::string sColor = (node->color == Color::RED) ? "RED" : "BLACK";
        std::cout << node->key << "(" << sColor << ")" << std::endl;
        printTreeHelper(node->left, indent, false);
        printTreeHelper(node->right, indent, true);
    }
}

// Destructor Helper
template <typename Key, typename Value>
void deleteNodes(Node<Key, Value>* node) {
    if (node == nullptr) return;
    deleteNodes(node->left);
    deleteNodes(node->right);
    delete node;
}

template <typename Key, typename Value>
RBTree<Key, Value>::~RBTree() {
    deleteNodes(root);
}



template <typename Key, typename Value>
Value& RBTree<Key, Value>::operator[](const Key& key) {
    Node<Key, Value>* node = search(key);
    if (node != nullptr) {
        return node->value;
    }

    // Key not found, insert with default value
    Value default_value = Value();
    insert(key, default_value);
    node = search(key); // Re-search to get the new node
    return node->value;
}

}  // namespace container_internal
}  // namespace absl

#endif  // ABSL_CONTAINER_INTERNAL_RBTREE_H_

/*
// Example Usage:
int main() {
    absl::container_internal::RBTree<int, std::string> tree;
    tree.insert(7, "seven");
    tree.insert(3, "three");
    tree.insert(18, "eighteen");
    tree.insert(10, "ten");
    tree.insert(22, "twenty-two");
    tree.insert(8, "eight");
    tree.insert(11, "eleven");
    tree.insert(26, "twenty-six");
    tree.insert(2, "two");
    tree.insert(6, "six");
    tree.insert(13, "thirteen");

    tree.printTree();
    std::cout << "--------------------" << std::endl;

    tree.deleteNode(18);
    tree.printTree();
    std::cout << "--------------------" << std::endl;

    tree.deleteNode(11);
    tree.printTree();
    std::cout << "--------------------" << std::endl;

    tree.deleteNode(3);
    tree.printTree();
    std::cout << "--------------------" << std::endl;

    auto* node = tree.search(10);
    if (node) {
        std::cout << "Found 10: " << node->value << std::endl;
    } else {
        std::cout << "10 not found" << std::endl;
    }

    std::cout << "--- List Traversal Forward --- " << std::endl;
    auto* curr = tree.getRoot() ? tree.getRoot()->min_node : nullptr;
    while (curr != nullptr) {
        std::cout << curr->key << " ";
        curr = curr->next;
    }
    std::cout << std::endl;

    std::cout << "--- List Traversal Backward --- " << std::endl;
    curr = tree.getRoot() ? tree.getRoot()->max_node : nullptr;
    while (curr != nullptr) {
        std::cout << curr->key << " ";
        curr = curr->prev;
    }
    std::cout << std::endl;

    return 0;
}
*/
