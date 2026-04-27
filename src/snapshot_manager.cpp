#include "snapshot_manager.h"
#include "storage_engine.h"
#include <fstream>

SnapshotManager::SnapshotManager(const std::string& file)
    : snapshotFile(file)
{}

void SnapshotManager::createSnapshot(StorageEngine& engine)
{
    engine.saveAll(snapshotFile);
}

void SnapshotManager::loadSnapshot(StorageEngine& engine)
{
    engine.loadAll(snapshotFile);
}