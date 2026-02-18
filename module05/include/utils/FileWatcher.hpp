#ifndef FILEWATCHER_HPP
#define FILEWATCHER_HPP

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <map>

// Watches a set of files for modification and fires a callback on change.
// Runs a background polling thread; safe to start/stop multiple times.
class FileWatcher
{
public:
    using Callback = std::function<void(const std::string&)>;

    // files         - absolute or relative paths to watch
    // callback      - called on the watcher thread when a file changes
    // pollIntervalMs - polling interval in milliseconds (default 500)
    FileWatcher(std::vector<std::string> files, Callback callback, int pollIntervalMs = 500);
    ~FileWatcher();

    FileWatcher(const FileWatcher&)            = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    void start();
    void stop();

private:
    std::vector<std::string>         _files;
    Callback                         _callback;
    int                              _pollIntervalMs;
    std::atomic<bool>                _running;
    std::thread                      _thread;
    std::map<std::string, long long> _lastModTimes;

    void      watchLoop();
    long long getModTime(const std::string& path) const;
};

#endif