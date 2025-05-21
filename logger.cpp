#include "logger.h"
#include <fstream>

Logger::Logger(const std::string& filename) : log_filename(filename), log_file(filename, std::ios::app) // std::ios::app - флаг, указывающий, что файл должен быть открыт в режиме добавления
{
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open log file");
    }
}

Logger::~Logger()
{
    if (log_file.is_open()) {
        log_file.close();
    }
}

std::string Logger::read_Log()
{
    mutex.lock_shared();

    std::ifstream input_file(log_filename);
    std::string line;
    if (input_file.is_open() && std::getline(input_file, line)) {
        mutex.unlock_shared();
        return line;
    }

    mutex.unlock_shared();
    return "";
}

void Logger::write_Log(const std::string& message)
{
    mutex.lock();
    if (log_file.is_open()) {
        log_file << message << std::endl;
    }
    mutex.unlock();
}
