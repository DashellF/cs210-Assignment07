#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

struct Node {
    int key;
    std::unique_ptr<Node> left, right;
    explicit Node(int value) : key(value) {}
};

class BinarySearchTree {
    using Ptr = std::unique_ptr<Node>;
    Ptr root;

    static bool insert(Ptr& node, int key) {
        if (!node) {
            node = std::make_unique<Node>(key);
            return true;
        }
        if (key < node->key) return insert(node->left, key);
        if (key > node->key) return insert(node->right, key);
        return false; // Reject duplicate keys.
    }

    static bool remove(Ptr& node, int key) {
        if (!node) return false;
        if (key < node->key) return remove(node->left, key);
        if (key > node->key) return remove(node->right, key);

        // Found the node. Handle zero or one child.
        if (!node->left) {
            Ptr replacement = std::move(node->right);
            node = std::move(replacement);
        } else if (!node->right) {
            Ptr replacement = std::move(node->left);
            node = std::move(replacement);
        } else {
            // Two children: use the inorder successor.
            const Node* successor = node->right.get();
            while (successor->left)
                successor = successor->left.get();
            node->key = successor->key;
            remove(node->right, node->key);
        }
        return true;
    }

    static void inorder(const Node* node, std::vector<int>& out) {
        if (!node) return;
        inorder(node->left.get(), out);
        out.push_back(node->key);
        inorder(node->right.get(), out);
    }

    static void preorder(const Node* node, std::vector<int>& out) {
        if (!node) return;
        out.push_back(node->key);
        preorder(node->left.get(), out);
        preorder(node->right.get(), out);
    }

    static void postorder(const Node* node, std::vector<int>& out) {
        if (!node) return;
        postorder(node->left.get(), out);
        postorder(node->right.get(), out);
        out.push_back(node->key);
    }

    static int height(const Node* node) {
        if (!node) return -1; // Height measured in edges.
        return 1 + std::max(height(node->left.get()),
                            height(node->right.get()));
    }

public:
    bool insert(int key) { return insert(root, key); }
    bool remove(int key) { return remove(root, key); }

    bool search(int key) const {
        const Node* current = root.get();
        while (current) {
            if (key == current->key) return true;
            current = key < current->key ? current->left.get()
                                         : current->right.get();
        }
        return false;
    }

    std::vector<int> inorder() const {
        std::vector<int> out;
        inorder(root.get(), out);
        return out;
    }

    std::vector<int> preorder() const {
        std::vector<int> out;
        preorder(root.get(), out);
        return out;
    }

    std::vector<int> postorder() const {
        std::vector<int> out;
        postorder(root.get(), out);
        return out;
    }

    int height() const { return height(root.get()); }
};

void print(const char* label, const std::vector<int>& values) {
    std::cout << label;
    for (int value : values) std::cout << ' ' << value;
    std::cout << '\n';
}

void checkRemoval(BinarySearchTree& tree, int key, const char* type,
                  const std::vector<int>& expected) {
    std::cout << "\nRemove " << key << " (" << type << ")\n";
    bool removed = tree.remove(key);
    assert(removed && !tree.search(key));
    auto values = tree.inorder();
    bool sorted = std::is_sorted(values.begin(), values.end());
    assert(sorted && values == expected);
    print("Inorder:", values);
    std::cout << "Sorted: " << sorted << '\n';
}

int main() {
    BinarySearchTree tree;
    const std::vector<int> keys =
        {50, 30, 70, 20, 40, 60, 80, 35, 45, 65, 75};
    for (int key : keys) {
        bool inserted = tree.insert(key);
        assert(inserted);
    }
    std::cout << std::boolalpha;
    print("Inserted:", keys);
    print("Inorder:", tree.inorder());
    print("Preorder:", tree.preorder());
    print("Postorder:", tree.postorder());
    std::cout << "Height (edges): " << tree.height() << '\n';
    assert(tree.height() == 3);
    assert(tree.inorder() == (std::vector<int>
        {20,30,35,40,45,50,60,65,70,75,80}));
    assert(tree.preorder() == (std::vector<int>
        {50,30,20,40,35,45,70,60,65,80,75}));
    assert(tree.postorder() == (std::vector<int>
        {20,35,45,40,30,65,60,75,80,70,50}));

    std::cout << "Search 65: " << tree.search(65) << '\n';
    std::cout << "Search 99: " << tree.search(99) << '\n';
    assert(tree.search(65) && !tree.search(99));

    checkRemoval(tree, 20, "leaf",
        {30,35,40,45,50,60,65,70,75,80});
    checkRemoval(tree, 60, "one child",
        {30,35,40,45,50,65,70,75,80});
    checkRemoval(tree, 50, "two children (root)",
        {30,35,40,45,65,70,75,80});
    std::cout << "Final height (edges): " << tree.height() << '\n';
    assert(tree.height() == 3);

    // Extra edge cases: duplicate, missing key, empty and root removal.
    bool duplicate = tree.insert(65);
    bool missing = tree.remove(99);
    assert(!duplicate && !missing);
    BinarySearchTree empty;
    assert(empty.height() == -1 && empty.inorder().empty());
    assert(!empty.search(1));
    bool absent = empty.remove(1);
    assert(!absent);
    empty.insert(10);
    assert(empty.height() == 0);
    empty.insert(5);
    bool rootRemoved = empty.remove(10);
    assert(rootRemoved && empty.search(5) && empty.height() == 0);
    rootRemoved = empty.remove(5);
    assert(rootRemoved && empty.height() == -1);
    std::cout << "\nAll assertions passed.\n";
}
