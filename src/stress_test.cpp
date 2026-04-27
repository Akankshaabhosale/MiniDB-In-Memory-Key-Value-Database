#include "storage_engine.h"
#include <thread>
#include <vector>
#include <string>

void worker(StorageEngine& engine, int id) {
    for (int i = 0; i < 1000; ++i) {
        std::string key = "key" + std::to_string(i);
        std::string value = "val" + std::to_string(id);

        engine.set(key, value);

        std::string result;
        engine.get(key, result);  // <-- correct usage
    }
}

int main() {
    StorageEngine engine;

    std::vector<std::thread> threads;

    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(worker, std::ref(engine), i);
    }

    for (auto& t : threads)
        t.join();

    return 0;
}