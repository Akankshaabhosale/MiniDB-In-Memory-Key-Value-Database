#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <shared_mutex>

class HashTable {
private:
    static const int TABLE_SIZE = 101;
    static const int MAX_CAPACITY = 5;

    struct Node {
        std::string key;
        std::string value;

        bool hasExpiry;
        std::chrono::steady_clock::time_point expiry;

        Node* next;
        Node* prevLRU;
        Node* nextLRU;

        Node(const std::string& k, const std::string& v)
            : key(k), value(v),
              hasExpiry(false),
              next(nullptr),
              prevLRU(nullptr),
              nextLRU(nullptr) {}
    };

    std::vector<Node*> table;

    Node* head;
    Node* tail;

    int currentSize;

    std::atomic<bool> running;
    std::thread cleanerThread;
    std::shared_mutex mtx;

    int hashFunction(const std::string& key);

    void addToFront(Node* node);
    void removeFromLRU(Node* node);
    void moveToFront(Node* node);
    void evictLRU();
    void cleanExpiredKeys();

public:
    HashTable();
    ~HashTable();

    void set(const std::string& key,
             const std::string& value,
             int ttlSeconds = 0);

    bool get(const std::string& key,
             std::string& value);

    bool remove(const std::string& key);

    void saveToDisk(const std::string& filename);
    void loadFromDisk(const std::string& filename);
};

#endif