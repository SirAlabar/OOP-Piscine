#include "utils/FileWatcher.hpp"
#include <sys/stat.h>
#include <chrono>
#include <thread>

FileWatcher::FileWatcher(std::vector<std::string> files, Callback callback, int pollIntervalMs)
    : _files(std::move(files)),
      _callback(std::move(callback)),
      _pollIntervalMs(pollIntervalMs),
      _running(false)
{
    // Snapshot initial modification times so the first poll has a baseline
    for (const auto& file : _files)
    {
        _lastModTimes[file] = getModTime(file);
    }
}

FileWatcher::~FileWatcher()
{
    stop();
}


// Public control
void FileWatcher::start()
{
    if (_running.load())
    {
        return;
    }

    _running.store(true);
    _thread = std::thread(&FileWatcher::watchLoop, this);
}

void FileWatcher::stop()
{
    _running.store(false);

    if (_thread.joinable())
    {
        _thread.join();
    }
}

// Private
long long FileWatcher::getModTime(const std::string& path) const
{
    struct stat st;

    if (stat(path.c_str(), &st) == 0)
    {
        return static_cast<long long>(st.st_mtime);
    }

    return -1LL;
}

void FileWatcher::watchLoop()
{
    while (_running.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_pollIntervalMs));

        for (const auto& file : _files)
        {
            long long currentMod = getModTime(file);
            long long lastMod    = _lastModTimes.at(file);

            if (currentMod == -1LL || currentMod == lastMod)
            {
                continue;
            }

            // Update recorded time immediately to suppress duplicate triggers
            _lastModTimes[file] = currentMod;

            // Debounce: wait briefly so the writer can finish flushing the file
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            if (_callback)
            {
                _callback(file);
            }
        }
    }
}