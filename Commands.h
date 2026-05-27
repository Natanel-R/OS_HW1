// Ver: 04-11-2025
#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
    // TODO: Add your data members
public:
    Command(const char* cmd_line);

    virtual ~Command();

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char* cmd_line);

    virtual ~BuiltInCommand() {
    }
};

class ExternalCommand : public Command {
    std::string cmd_line;
    bool isComplexCommand(const std::string& cmd);

public:
    ExternalCommand(const char* cmd_line);

    virtual ~ExternalCommand() {
    }

    void execute() override;
};


class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line);

    virtual ~RedirectionCommand() {
    }

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char* cmd_line);

    virtual ~PipeCommand() {
    }

    void execute() override;
};

class DiskUsageCommand : public Command {
public:
    DiskUsageCommand(const char* cmd_line);

    virtual ~DiskUsageCommand() {
    }

    void execute() override;
};

class WhoAmICommand : public Command {
public:
    WhoAmICommand(const char* cmd_line);

    virtual ~WhoAmICommand() {
    }

    void execute() override;
};

class USBInfoCommand : public Command {
    // TODO: Add your data members **BONUS: 10 Points**
public:
    USBInfoCommand(const char* cmd_line);

    virtual ~USBInfoCommand() {
    }

    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    ChangeDirCommand(const char* cmd_line, char** plastPwd);

    virtual ~ChangeDirCommand() {
    }

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char* cmd_line);

    virtual ~GetCurrDirCommand() {
    }

    void execute() override;
};

class ChpromptCommand : public BuiltInCommand {
private:
    std::string name = "smash";

public:
    ChpromptCommand(const char* cmd_line);

    virtual ~ChpromptCommand() {
    };
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char* cmd_line);

    virtual ~ShowPidCommand() {
    }

    void execute() override;
};

class PwdCommand : public BuiltInCommand {
public:
    PwdCommand(const char* cmd_line);

    virtual ~PwdCommand() {
    }

    void execute() override;
};

class CdCommand : public BuiltInCommand {
private:
    int numOfArgs;
    std::string newDir;

public:
    CdCommand(const char* cmd_line);

    virtual ~CdCommand() {
    }

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
private:
    bool kill_active;
    JobsList* jobs;

public:
    QuitCommand(const char* cmd_line, JobsList* jobs);

    virtual ~QuitCommand() {
    }

    void execute() override;
};

class JobsList {
public:
    class JobEntry {
    private:
        int jobId;
        int processId;
        std::string cmd_line;
        bool isStopped;

    public:
        JobEntry(int jobId, int processId, std::string cmd_line, bool stopped = false) :
            jobId(jobId), processId(processId), cmd_line(cmd_line), isStopped(stopped) {
        };
        int getJobId() const { return jobId; }
        int getProcessId() const { return processId; }
        std::string getCmd_line() const { return cmd_line; }
        bool getIsStopped() const { return isStopped; }
    };

private:
    std::map<int, JobEntry> jobs_map;
    int max_job_id;

public:
    JobsList();

    ~JobsList();

    void addJob(Command* cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry* getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry* getLastJob(int* lastJobId);

    ;
    JobEntry* getLastStoppedJob(int* jobId);

    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
private:
    JobsList* jobs;

public:
    JobsCommand(const char* cmd_line, JobsList* jobs);

    virtual ~JobsCommand() {
    }

    void execute() override;
};

class KillCommand : public BuiltInCommand {
private:
    JobsList* jobs;
    int signum;
    int jobId;
    bool valid;

public:
    KillCommand(const char* cmd_line, JobsList* jobs);

    virtual ~KillCommand() {
    }

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);

    virtual ~ForegroundCommand() {
    }

    void execute() override;
};

class AliasCommand : public BuiltInCommand {
private:
    std::string name;
    std::string command;
    bool is_print_only;
    bool valid;
    int error_type;

public:
    AliasCommand(const char* cmd_line);

    virtual ~AliasCommand() {
    }

    void execute() override;
};

class UnAliasCommand : public BuiltInCommand {
private:
    bool not_enough_args = false;
    std::vector<std::string> remove_aliases;

public:
    UnAliasCommand(const char* cmd_line);

    virtual ~UnAliasCommand() {
    }

    void execute() override;
};

class UnSetEnvCommand : public BuiltInCommand {
private:
    bool not_enough_args = false;
    std::vector<std::string> remove_envvar;

public:
    UnSetEnvCommand(const char* cmd_line);

    virtual ~UnSetEnvCommand() {
    }

    void execute() override;
};

class SysInfoCommand : public BuiltInCommand {
public:
    SysInfoCommand(const char* cmd_line);

    virtual ~SysInfoCommand() {
    }

    void execute() override;
};

class SmallShell {
private:
    // TODO: Add your data members
    std::string prompt = "smash";
    JobsList jobs;
    std::vector<std::pair<std::string, std::string>> aliases;
    SmallShell();
    std::string lastDir = "";

public:
    Command* CreateCommand(const char* cmd_line);

    SmallShell(SmallShell const&) = delete; // disable copy ctor
    void operator=(SmallShell const&) = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    std::string getPrompt() const;

    void setPrompt(const std::string& newPrompt);

    std::string getLastDir() const;

    void setLastDir(const std::string& newLastDir);

    JobsList* getJobslist() { return &jobs; }

    std::vector<std::pair<std::string, std::string>>* getAliases() { return &aliases; }

    ~SmallShell();

    void executeCommand(const char* cmd_line);

    // TODO: add extra methods as needed
};


#endif //SMASH_COMMAND_H_
