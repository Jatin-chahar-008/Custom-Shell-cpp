#include <utility>
#include <functional>
#include <stdexcept>
#include <string>
#include "vector.hpp"
#include <initializer_list>

template<typename Key, typename Value, typename Compare = std::less<Key>>
class Map {
private:
    struct Node {
        std::pair<const Key, Value> data;
        Node* left;
        Node* right;
        Node* parent;
        
        template<typename K, typename V>
        Node(K&& key, V&& value, Node* parent = nullptr)
            : data(std::forward<K>(key), std::forward<V>(value)), 
              left(nullptr), right(nullptr), parent(parent) {}
            
        template<typename P>
        Node(P&& pair, Node* parent = nullptr)
            : data(std::forward<P>(pair)), 
              left(nullptr), right(nullptr), parent(parent) {}
    };
    
    Node* root;
    size_t node_count;
    Compare comp;
    
public:
    class Iterator {
    private:
        Node* current;
        
        // Find the minimum node in a subtree
        Node* findMin(Node* node) const {
            if (!node) return nullptr;
            while (node->left) {
                node = node->left;
            }
            return node;
        }
        
        // Find the successor of a node
        Node* findSuccessor(Node* node) const {
            if (!node) return nullptr;
            
            // If right subtree exists, find minimum in right subtree
            if (node->right) {
                return findMin(node->right);
            }
            
            // Otherwise, go up until we find a parent for which current node is in left subtree
            Node* parent = node->parent;
            while (parent && node == parent->right) {
                node = parent;
                parent = parent->parent;
            }
            return parent;
        }
        
    public:
        Iterator(Node* node = nullptr) : current(node) {}
        
        std::pair<const Key, Value>& operator*() const {
            return current->data;
        }
        
        std::pair<const Key, Value>* operator->() const {
            return &(current->data);
        }
        
        Iterator& operator++() {
            current = findSuccessor(current);
            return *this;
        }
        
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        bool operator==(const Iterator& other) const {
            return current == other.current;
        }
        
        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
        
        friend class Map;
    };
    
    // Default constructor
    Map() : root(nullptr), node_count(0), comp(Compare()) {}
    
    // Initializer list constructor
    Map(std::initializer_list<std::pair<const Key, Value>> init) 
        : root(nullptr), node_count(0), comp(Compare()) {
        for (const auto& pair : init) {
            insert(pair);
        }
    }
    
    // Destructor
    ~Map() {
        clear();
    }
    
    // Copy constructor
    Map(const Map& other) : root(nullptr), node_count(0), comp(other.comp) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            insert(*it);
        }
    }
    
    // Move constructor
    Map(Map&& other) noexcept : root(other.root), node_count(other.node_count), comp(std::move(other.comp)) {
        other.root = nullptr;
        other.node_count = 0;
    }
    
    // Copy assignment
    Map& operator=(const Map& other) {
        if (this != &other) {
            clear();
            comp = other.comp;
            for (auto it = other.begin(); it != other.end(); ++it) {
                insert(*it);
            }
        }
        return *this;
    }
    
    // Move assignment
    Map& operator=(Map&& other) noexcept {
        if (this != &other) {
            clear();
            root = other.root;
            node_count = other.node_count;
            comp = std::move(other.comp);
            
            other.root = nullptr;
            other.node_count = 0;
        }
        return *this;
    }
    
    // Initializer list assignment
    Map& operator=(std::initializer_list<std::pair<const Key, Value>> ilist) {
        clear();
        for (const auto& pair : ilist) {
            insert(pair);
        }
        return *this;
    }
    
    Iterator begin() const {
        return Iterator(findMin(root));
    }
    
    Iterator end() const {
        return Iterator(nullptr);
    }
    
    size_t size() const {
        return node_count;
    }
    
    bool empty() const {
        return node_count == 0;
    }
    
    void clear() {
        clearSubtree(root);
        root = nullptr;
        node_count = 0;
    }
    
    Iterator find(const Key& key) const {
        Node* node = root;
        while (node) {
            if (comp(key, node->data.first)) {
                node = node->left;
            } else if (comp(node->data.first, key)) {
                node = node->right;
            } else {
                return Iterator(node);
            }
        }
        return end();
    }
    
    // Perfect forwarding insert for pair
    template<typename P>
    std::pair<Iterator, bool> insert(P&& pair) {
        return insertInternal(std::forward<P>(pair));
    }
    
    // Insert for key-value pair
    template<typename K, typename V>
    std::pair<Iterator, bool> insert(K&& key, V&& value) {
        return insertInternal(std::make_pair(std::forward<K>(key), std::forward<V>(value)));
    }
    
    // Emplace implementation using perfect forwarding
    template<typename... Args>
    std::pair<Iterator, bool> emplace(Args&&... args) {
        return insertInternal(std::pair<const Key, Value>(std::forward<Args>(args)...));
    }
    
    // Subscript operator for lvalue keys
    Value& operator[](const Key& key) {
        Iterator it = find(key);
        if (it != end()) {
            return it->second;
        }
        
        auto result = insert(key, Value());
        return result.first->second;
    }
    
    // Subscript operator for rvalue keys
    Value& operator[](Key&& key) {
        Iterator it = find(key);
        if (it != end()) {
            return it->second;
        }
        
        auto result = insert(std::move(key), Value());
        return result.first->second;
    }
    
    Value& at(const Key& key) {
        Iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("Map::at: key not found");
        }
        return it->second;
    }
    
    const Value& at(const Key& key) const {
        Iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("Map::at: key not found");
        }
        return it->second;
    }
    
    size_t erase(const Key& key) {
        Node* current = root;
        
        // Find the node to remove
        while (current) {
            if (comp(key, current->data.first)) {
                current = current->left;
            } else if (comp(current->data.first, key)) {
                current = current->right;
            } else {
                break;
            }
        }
        
        if (!current) {
            return 0; // Key not found
        }
        
        eraseNode(current);
        --node_count;
        return 1;
    }

    size_t count(const Key& key) const {
        return find(key) != end() ? 1 : 0;
    }
    
private:
    Node* findMin(Node* node) const {
        if (!node) return nullptr;
        while (node->left) {
            node = node->left;
        }
        return node;
    }
    
    void clearSubtree(Node* node) {
        if (!node) return;
        
        clearSubtree(node->left);
        clearSubtree(node->right);
        delete node;
    }
    
    // Internal insertion helper with perfect forwarding
    template<typename P>
    std::pair<Iterator, bool> insertInternal(P&& pair) {
        if (!root) {
            root = new Node(std::forward<P>(pair));
            node_count = 1;
            return std::make_pair(Iterator(root), true);
        }
        
        Node* current = root;
        Node* parent = nullptr;
        
        // Use the first element of the pair for comparison
        const Key& key = pair.first;
        
        while (current) {
            parent = current;
            
            if (comp(key, current->data.first)) {
                current = current->left;
            } else if (comp(current->data.first, key)) {
                current = current->right;
            } else {
                // Key already exists
                return std::make_pair(Iterator(current), false);
            }
        }
        
        Node* new_node = new Node(std::forward<P>(pair), parent);
        
        if (comp(key, parent->data.first)) {
            parent->left = new_node;
        } else {
            parent->right = new_node;
        }
        
        node_count++;
        return std::make_pair(Iterator(new_node), true);
    }
    
    void eraseNode(Node* node) {
        // Case 1: Node has no children
        if (!node->left && !node->right) {
            if (node == root) {
                root = nullptr;
            } else {
                Node* parent = node->parent;
                if (parent->left == node) {
                    parent->left = nullptr;
                } else {
                    parent->right = nullptr;
                }
            }
            delete node;
        }
        // Case 2: Node has one child
        else if (!node->left) {
            Node* right = node->right;
            right->parent = node->parent;
            
            if (node == root) {
                root = right;
            } else {
                Node* parent = node->parent;
                if (parent->left == node) {
                    parent->left = right;
                } else {
                    parent->right = right;
                }
            }
            delete node;
        }
        else if (!node->right) {
            Node* left = node->left;
            left->parent = node->parent;
            
            if (node == root) {
                root = left;
            } else {
                Node* parent = node->parent;
                if (parent->left == node) {
                    parent->left = left;
                } else {
                    parent->right = left;
                }
            }
            delete node;
        }
        // Case 3: Node has two children
        else {
            // Find the successor (minimum value in right subtree)
            Node* successor = findMin(node->right);
            
            // Copy successor's key (const_cast is safe here since we're replacing the node anyway)
            const_cast<Key&>(node->data.first) = successor->data.first;
            
            // Copy successor's value using assignment
            node->data.second = successor->data.second;
            
            // Recursively delete the successor
            eraseNode(successor);
            // Adjust the node count to avoid double-counting
            ++node_count;
        }
    }
};