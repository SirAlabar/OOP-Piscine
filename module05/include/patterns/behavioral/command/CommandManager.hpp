#ifndef COMMANDMANAGER_HPP
#define COMMANDMANAGER_HPP

#include <string>
#include <vector>

class ICommand;

// Metadata written to and read from a replay file.
struct RecordingMetadata
{
    std::string  networkFile;
    std::string  trainFile;
    unsigned int seed     = 0;
    double       stopTime = 0.0;  // sim time (seconds) when recording stopped
};

// Owns a list of ICommand objects.
// In recording mode:  Application calls record() after each action.
// In replay mode:     SimulationManager calls getCommandsForTime() each tick.
class CommandManager
{
public:
    CommandManager();
    ~CommandManager();

    CommandManager(const CommandManager&)            = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    // --- Recording ---
    void startRecording();
    bool isRecording() const;

    // Takes ownership of cmd and appends it to the log.
    void record(ICommand* cmd);

    // --- Replay ---
    void startReplay();
    bool isReplaying() const;

    // Returns (non-owning) pointers to commands whose timestamp falls in
    // [startTime, endTime).  Caller must not delete the returned pointers.
    std::vector<ICommand*> getCommandsForTime(double startTime, double endTime) const;

    // --- Persistence ---
    bool saveToFile(const std::string& path, const RecordingMetadata& meta) const;
    bool loadFromFile(const std::string& path, RecordingMetadata& outMeta);

    // --- Query ---
    std::size_t commandCount() const;

private:
    std::vector<ICommand*> _commands;
    bool                   _recording;
    bool                   _replaying;

    // Reconstruct a single command object from its serialized JSON line.
    // Returns nullptr when the type is unrecognised or the line is malformed.
    static ICommand* deserializeCommand(const std::string& json);

    // Minimal JSON helpers — no external dependency.
    static std::string extractString(const std::string& json, const std::string& key);
    static double      extractDouble(const std::string& json, const std::string& key);
    static long long   extractInt(const std::string& json, const std::string& key);
};

#endif