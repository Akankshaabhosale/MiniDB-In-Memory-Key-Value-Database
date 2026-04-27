#include "write_ahead_log.h"
#include <sstream>

WriteAheadLog::WriteAheadLog(const std::string& file)
    : filename(file)
{}

void WriteAheadLog::appendSet(const std::string& key,
                              const std::string& value,
                              int ttl)
{
    std::lock_guard<std::mutex> lock(walMutex);
    std::ofstream out(filename, std::ios::app);
    out << "SET " << key << " " << value << " " << ttl << "\n";
}

void WriteAheadLog::appendDelete(const std::string& key)
{
    std::lock_guard<std::mutex> lock(walMutex);
    std::ofstream out(filename, std::ios::app);
    out << "DEL " << key << "\n";
}

std::vector<std::string> WriteAheadLog::readAll()
{
    std::lock_guard<std::mutex> lock(walMutex);

    std::vector<std::string> lines;
    std::ifstream in(filename);

    std::string line;
    while (std::getline(in, line))
        lines.push_back(line);

    return lines;
}

void WriteAheadLog::clear()
{
    std::lock_guard<std::mutex> lock(walMutex);
    std::ofstream out(filename, std::ios::trunc);
}