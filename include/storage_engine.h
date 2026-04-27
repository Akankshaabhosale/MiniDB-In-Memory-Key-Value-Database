#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include "hash_table.h"
#include "write_ahead_log.h"
#include "snapshot_manager.h"
#include <vector>
#include <memory>

class StorageEngine {
private:
    int shardCount;
    std::vector<std::unique_ptr<HashTable>> shards;

    WriteAheadLog wal;
    SnapshotManager snapshot;

    int getShardIndex(const std::string& key);

public:
    StorageEngine(int shardCount = 4);

    void set(const std::string& key,
             const std::string& value,
             int ttl = 0);

    bool get(const std::string& key, std::string& value);

    void del(const std::string& key);

    void createSnapshot();

    void recover();

    void saveAll(const std::string& file);
    void loadAll(const std::string& file);
};

#endif