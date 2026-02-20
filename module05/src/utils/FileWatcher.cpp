#include "utils/FileWatcher.hpp"
#include <sys/stat.h>
#include <chrono>

FileWatcher::FileWatcher(std::vector<std::string> files, Callback callback, int pollIntervalMs)
    : _files(std::move(files)),
      _callback(std::move(callback)),
      _pollIntervalMs(pollIntervalMs),
      _running(false),
      _lastPollTimeMs(0)
{
    for (const auto& file : _files)
    {
        _lastModTimes[file] = getModTime(file);
    }
}

FileWatcher::~FileWatcher()
{
    stop();
}

void FileWatcher::start()
{
    _running = true;
    _lastPollTimeMs = nowMs();
}

void FileWatcher::stop()
{
    _running = false;
}

long long FileWatcher::nowMs() const
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

void FileWatcher::poll()
{
    if (!_running)
    {
        return;
    }

    const long long currentTime = nowMs();
    if (currentTime - _lastPollTimeMs < _pollIntervalMs)
    {
        return;
    }

    _lastPollTimeMs = currentTime;

    for (const auto& file : _files)
    {
        long long currentMod = getModTime(file);
        long long lastMod    = _lastModTimes.at(file);

        if (currentMod == -1LL || currentMod == lastMod)
        {
            continue;
        }

        _lastModTimes[file] = currentMod;

        if (_callback)
        {
            _callback(file);
        }
    }
}

long long FileWatcher::getModTime(const std::string& path) const
{
    struct stat st;

    if (stat(path.c_str(), &st) == 0)
    {
        return static_cast<long long>(st.st_mtime);
    }

    return -1LL;
}
