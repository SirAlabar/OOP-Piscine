#ifndef FILEWATCHER_HPP
#define FILEWATCHER_HPP

#include <string>
#include <vector>
#include <functional>
#include <map>

// Watches a set of files for modification and fires a callback on change.
// Polling is explicit and runs on the caller's thread.
class FileWatcher
{
public:
    using Callback = std::function<void(const std::string&)>;

    FileWatcher(std::vector<std::string> files, Callback callback, int pollIntervalMs = 500);
    ~FileWatcher();

    FileWatcher(const FileWatcher&)            = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    void start();
    void stop();
    void poll();

private:
    std::vector<std::string>         _files;
    Callback                         _callback;
    int                              _pollIntervalMs;
    bool                             _running;
    std::map<std::string, long long> _lastModTimes;
    long long                        _lastPollTimeMs;

    long long getModTime(const std::string& path) const;
    long long nowMs() const;
};

#endif
