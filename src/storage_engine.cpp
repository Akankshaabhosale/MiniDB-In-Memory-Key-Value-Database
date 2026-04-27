#include "storage_engine.h"
#include <fstream>
#include <sstream>

StorageEngine::StorageEngine(int count)
    : shardCount(count),
      wal("wal.log"),
      snapshot("snapshot.dat")
{
    for (int i = 0; i < shardCount; i++)
        shards.push_back(std::make_unique<HashTable>());

    recover();
}

int StorageEngine::getShardIndex(const std::string& key)
{
    std::hash<std::string> hasher;
    return hasher(key) % shardCount;
}

void StorageEngine::set(const std::string& key,
                        const std::string& value,
                        int ttl)
{
    wal.appendSet(key, value, ttl);

    int index = getShardIndex(key);
    shards[index]->set(key, value, ttl);
}

bool StorageEngine::get(const std::string& key, std::string& value)
{
    int index = getShardIndex(key);
    return shards[index]->get(key, value);
}

void StorageEngine::del(const std::string& key)
{
    wal.appendDelete(key);

    int index = getShardIndex(key);
    shards[index]->remove(key);
}

void StorageEngine::createSnapshot()
{
    snapshot.createSnapshot(*this);
    wal.clear();
}

void StorageEngine::recover()
{
    // 1. Load snapshot first
    snapshot.loadSnapshot(*this);

    // 2. Replay WAL without re-appending
    auto lines = wal.readAll();

    for (auto& line : lines)
    {
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "SET")
        {
            std::string key, value;
            int ttl;
            iss >> key >> value >> ttl;

            int index = getShardIndex(key);
            shards[index]->set(key, value, ttl);
        }
        else if (cmd == "DEL")
        {
            std::string key;
            iss >> key;

            int index = getShardIndex(key);
            shards[index]->remove(key);
        }
    }
}

void StorageEngine::saveAll(const std::string& file)
{
    for (int i = 0; i < shards.size(); ++i)
    {
        shards[i]->saveToDisk(file + "_shard_" + std::to_string(i));
    }
}

void StorageEngine::loadAll(const std::string& file)
{
    for (int i = 0; i < shards.size(); ++i)
    {
        shards[i]->loadFromDisk(file + "_shard_" + std::to_string(i));
    }
}