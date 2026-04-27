#ifndef SNAPSHOT_MANAGER_H
#define SNAPSHOT_MANAGER_H

#include <string>

class StorageEngine;

class SnapshotManager {
private:
    std::string snapshotFile;

public:
    SnapshotManager(const std::string& file);

    void createSnapshot(StorageEngine& engine);
    void loadSnapshot(StorageEngine& engine);
};

#endif