#ifndef COMMANDMANAGER_HPP
#define COMMANDMANAGER_HPP

#include <cstddef>
#include <string>
#include <vector>

class ICommand;

// Metadata stored at the top of every replay JSON file.
struct RecordingMetadata
{
    std::string  networkFile;
    std::string  trainFile;
    unsigned int seed = 0;
};

// Manages command recording, persistence, replay.
//
// RECORD mode  — SimulationManager calls record() at key events.
//                Application calls saveToFile() on shutdown.
//
// REPLAY mode  — Application calls loadFromFile(); CommandManager owns
//                the command list and serves them via getCommandsForTime().
class CommandManager
{
public:
    enum class Mode { IDLE, RECORD, REPLAY };

    CommandManager();
    ~CommandManager();

    CommandManager(const CommandManager&)            = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    // ── Record ─────────────────────────────────────────────────────────────
    void startRecording();

    // Takes ownership of cmd.  Ignored if not in RECORD mode.
    void record(ICommand* cmd);

    // Serialise all recorded commands to a JSON file.
    void saveToFile(const std::string& path, const RecordingMetadata& meta) const;

    // ── Replay ─────────────────────────────────────────────────────────────
    // Parse a previously saved JSON file.
    // Populates outMeta with the recorded header data.
    // Returns false and leaves the manager in IDLE if parsing fails.
    bool loadFromFile(const std::string& path, RecordingMetadata& outMeta);

    void startReplay();

    // Returns all commands whose timestamp t satisfies tFrom <= t < tTo.
    // Advances the internal replay cursor; each command is returned exactly once.
    std::vector<ICommand*> getCommandsForTime(double tFrom, double tTo);

    // ── Queries ────────────────────────────────────────────────────────────
    Mode        getMode()      const;
    bool        isRecording()  const;
    bool        isReplaying()  const;
    std::size_t commandCount() const;

private:
    Mode                   _mode;
    std::vector<ICommand*> _commands;
    std::size_t            _replayIndex;

    // Parse a single JSON command object line into a key→value map.
    // All values are returned as unquoted strings.
    using KVMap = std::vector<std::pair<std::string, std::string>>;
    static KVMap     parseJsonObject(const std::string& line);

    // Factory: create a concrete ICommand from a parsed KVMap.
    // Returns nullptr for unknown types.
    static ICommand* createCommandFromKV(const KVMap& kv);
};

#endif