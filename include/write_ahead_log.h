#ifndef WRITE_AHEAD_LOG_H
#define WRITE_AHEAD_LOG_H

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

class WriteAheadLog {
private:
    std::string filename;
    std::mutex walMutex;

public:
    WriteAheadLog(const std::string& file);

    void appendSet(const std::string& key,
                   const std::string& value,
                   int ttl);

    void appendDelete(const std::string& key);

    std::vector<std::string> readAll();

    void clear();
};

#endif