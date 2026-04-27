
#include "hash_table.h"

#include <fstream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>

HashTable::HashTable()
    : head(nullptr), tail(nullptr),
      currentSize(0), running(true)
{
    table.resize(TABLE_SIZE, nullptr);

    cleanerThread = std::thread(&HashTable::cleanExpiredKeys, this);
}

HashTable::~HashTable()
{
    running = false;

    if (cleanerThread.joinable())
        cleanerThread.join();

    std::unique_lock<std::shared_mutex> lock(mtx);

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node* current = table[i];
        while (current)
        {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
    }
}

int HashTable::hashFunction(const std::string& key)
{
    unsigned long hash = 5381;
    for (char c : key)
        hash = ((hash << 5) + hash) + c; // djb2
    return hash % TABLE_SIZE;
}

void HashTable::addToFront(Node* node)
{
    node->prevLRU = nullptr;
    node->nextLRU = head;

    if (head)
        head->prevLRU = node;

    head = node;

    if (!tail)
        tail = node;
}

void HashTable::removeFromLRU(Node* node)
{
    if (node->prevLRU)
        node->prevLRU->nextLRU = node->nextLRU;
    else
        head = node->nextLRU;

    if (node->nextLRU)
        node->nextLRU->prevLRU = node->prevLRU;
    else
        tail = node->prevLRU;
}

void HashTable::moveToFront(Node* node)
{
    removeFromLRU(node);
    addToFront(node);
}

void HashTable::evictLRU()
{
    if (!tail)
        return;

    Node* node = tail;
    int index = hashFunction(node->key);

    Node* curr = table[index];
    Node* prev = nullptr;

    while (curr)
    {
        if (curr == node)
        {
            if (prev)
                prev->next = curr->next;
            else
                table[index] = curr->next;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    removeFromLRU(node);
    delete node;
    currentSize--;
}

void HashTable::set(const std::string& key,
                    const std::string& value,
                    int ttlSeconds)
{
    std::unique_lock<std::shared_mutex> lock(mtx);

    int index = hashFunction(key);
    Node* curr = table[index];

    while (curr)
    {
        if (curr->key == key)
        {
            curr->value = value;

            if (ttlSeconds > 0)
            {
                curr->hasExpiry = true;
                curr->expiry = std::chrono::steady_clock::now()
                               + std::chrono::seconds(ttlSeconds);
            }

            moveToFront(curr);
            return;
        }
        curr = curr->next;
    }

    if (currentSize >= MAX_CAPACITY)
        evictLRU();

    Node* newNode = new Node(key, value);

    if (ttlSeconds > 0)
    {
        newNode->hasExpiry = true;
        newNode->expiry = std::chrono::steady_clock::now()
                          + std::chrono::seconds(ttlSeconds);
    }

    newNode->next = table[index];
    table[index] = newNode;

    addToFront(newNode);
    currentSize++;
}

bool HashTable::get(const std::string& key, std::string& value)
{
    std::unique_lock<std::shared_mutex> lock(mtx);

    int index = hashFunction(key);
    Node* curr = table[index];
    Node* prev = nullptr;

    while (curr)
    {
        if (curr->key == key)
        {
            if (curr->hasExpiry &&
                std::chrono::steady_clock::now() > curr->expiry)
            {
                // remove expired
                if (prev)
                    prev->next = curr->next;
                else
                    table[index] = curr->next;

                removeFromLRU(curr);
                delete curr;
                currentSize--;
                return false;
            }

            value = curr->value;
            moveToFront(curr);
            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

bool HashTable::remove(const std::string& key)
{
    std::unique_lock<std::shared_mutex> lock(mtx);

    int index = hashFunction(key);
    Node* curr = table[index];
    Node* prev = nullptr;

    while (curr)
    {
        if (curr->key == key)
        {
            if (prev)
                prev->next = curr->next;
            else
                table[index] = curr->next;

            removeFromLRU(curr);
            delete curr;
            currentSize--;
            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

void HashTable::cleanExpiredKeys()
{
    while (running)
    {
        {
            std::unique_lock<std::shared_mutex> lock(mtx);

            for (int i = 0; i < TABLE_SIZE; i++)
            {
                Node* curr = table[i];
                Node* prev = nullptr;

                while (curr)
                {
                    if (curr->hasExpiry &&
                        std::chrono::steady_clock::now() > curr->expiry)
                    {
                        Node* temp = curr;

                        if (prev)
                            prev->next = curr->next;
                        else
                            table[i] = curr->next;

                        removeFromLRU(temp);
                        curr = curr->next;

                        delete temp;
                        currentSize--;
                    }
                    else
                    {
                        prev = curr;
                        curr = curr->next;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void HashTable::saveToDisk(const std::string& filename)
{
    std::unique_lock<std::shared_mutex> lock(mtx);

    std::ofstream out(filename);

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        Node* curr = table[i];

        while (curr)
        {
            out << curr->key << " "
                << curr->value << " "
                << curr->hasExpiry << " ";

            if (curr->hasExpiry)
            {
                auto remaining =
                    std::chrono::duration_cast<std::chrono::seconds>(
                        curr->expiry - std::chrono::steady_clock::now()
                    ).count();

                out << remaining;
            }
            else
            {
                out << 0;
            }

            out << "\n";
            curr = curr->next;
        }
    }
}

void HashTable::loadFromDisk(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in.is_open())
        return;

    std::string key, value;
    bool hasExpiry;
    int ttl;

    while (in >> key >> value >> hasExpiry >> ttl)
    {
        if (hasExpiry && ttl > 0)
            set(key, value, ttl);
        else
            set(key, value);
    }
}