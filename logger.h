#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <shared_mutex>
#include <string>

class Logger {
public:
    explicit Logger(const std::string& filename);
    ~Logger();
    void write_Log(const std::string& message);
    std::string read_Log();

private:
    std::string log_filename;
    std::fstream log_file;
    std::shared_mutex mutex;
};

#endif 
