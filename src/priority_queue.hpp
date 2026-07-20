#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 *
 * Implements a leftist heap for O(log n) merge complexity.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node *left;
        Node *right;
        int dist;  // Distance to nearest null descendant (null path length)

        Node(const T &d) : data(d), left(nullptr), right(nullptr), dist(0) {}
    };

    Node *root;
    size_t sz;
    Compare cmp;

    // Calculate distance of a node
    int getDist(Node *n) const {
        return n ? n->dist : -1;
    }

    // Update distance after children change
    void updateDist(Node *n) {
        if (n) {
            n->dist = getDist(n->right) + 1;
        }
    }

    // Merge two nodes - returns the new root
    Node* mergeNodes(Node *a, Node *b) {
        if (!a) return b;
        if (!b) return a;

        // Ensure a is the root with larger key (for max-heap)
        if (cmp(a->data, b->data)) {
            std::swap(a, b);
        }

        // Merge b into a's right subtree
        a->right = mergeNodes(a->right, b);

        // Update distance
        updateDist(a);

        // Leftist property: left child should have >= distance than right
        if (getDist(a->left) < getDist(a->right)) {
            std::swap(a->left, a->right);
        }
        updateDist(a);

        return a;
    }

    // Deep copy a subtree
    Node* copyTree(Node *n) {
        if (!n) return nullptr;
        Node *newNode = new Node(n->data);
        newNode->dist = n->dist;
        newNode->left = copyTree(n->left);
        newNode->right = copyTree(n->right);
        return newNode;
    }

    // Delete a subtree
    void deleteTree(Node *n) {
        if (!n) return;
        deleteTree(n->left);
        deleteTree(n->right);
        delete n;
    }

public:
    /**
     * @brief default constructor
     */
    priority_queue() : root(nullptr), sz(0) {}

    /**
     * @brief copy constructor
     * @param other the priority_queue to be copied
     */
    priority_queue(const priority_queue &other) : root(nullptr), sz(0) {
        root = copyTree(other.root);
        sz = other.sz;
    }

    /**
     * @brief deconstructor
     */
    ~priority_queue() {
        deleteTree(root);
    }

    /**
     * @brief Assignment operator
     * @param other the priority_queue to be assigned from
     * @return a reference to this priority_queue after assignment
     */
    priority_queue &operator=(const priority_queue &other) {
        if (this != &other) {
            // Exception-safe: create new tree first, then swap
            Node *newRoot = copyTree(other.root);
            deleteTree(root);
            root = newRoot;
            sz = other.sz;
        }
        return *this;
    }

    /**
     * @brief get the top element of the priority queue.
     * @return a reference of the top element.
     * @throws container_is_empty if empty() returns true
     */
    const T & top() const {
        if (empty()) {
            throw container_is_empty();
        }
        return root->data;
    }

    /**
     * @brief push new element to the priority queue.
     * @param e the element to be pushed
     */
    void push(const T &e) {
        // Create temporary node first
        Node *newNode = new Node(e);

        try {
            // Merge current heap with single node heap
            root = mergeNodes(root, newNode);
            sz++;
        } catch (...) {
            // If merge fails (cmp throws), clean up and rethrow
            delete newNode;
            throw runtime_error();
        }
    }

    /**
     * @brief delete the top element from the priority queue.
     * @throws container_is_empty if empty() returns true
     */
    void pop() {
        if (empty()) {
            throw container_is_empty();
        }

        // For exception safety, we need to save state that can be restored
        // But since pop only uses merge, and merge may throw from cmp,
        // we need to handle that case.

        Node *oldRoot = root;

        try {
            // Merge left and right subtrees
            root = mergeNodes(root->left, root->right);
            sz--;

            // Now safe to delete old root
            oldRoot->left = nullptr;
            oldRoot->right = nullptr;
            delete oldRoot;
        } catch (...) {
            // Restore original state
            throw runtime_error();
        }
    }

    /**
     * @brief return the number of elements in the priority queue.
     * @return the number of elements.
     */
    size_t size() const {
        return sz;
    }

    /**
     * @brief check if the container is empty.
     * @return true if it is empty, false otherwise.
     */
    bool empty() const {
        return sz == 0;
    }

    /**
     * @brief merge another priority_queue into this one.
     * The other priority_queue will be cleared after merging.
     * The complexity is at most O(logn).
     * @param other the priority_queue to be merged.
     */
    void merge(priority_queue &other) {
        if (this == &other) {
            return;
        }

        if (other.empty()) {
            return;
        }

        // Save other's state for potential restoration
        Node *otherRoot = other.root;
        size_t otherSz = other.sz;

        try {
            // Merge other's heap into this heap
            root = mergeNodes(root, other.root);
            sz += other.sz;

            // Clear other
            other.root = nullptr;
            other.sz = 0;
        } catch (...) {
            // Restore other and rethrow
            other.root = otherRoot;
            other.sz = otherSz;
            throw runtime_error();
        }
    }
};

}

#endif
