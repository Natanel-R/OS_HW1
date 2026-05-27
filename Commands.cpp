#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <regex>
#include <limits.h>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char*)malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;
    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() {
    // TODO: add your implementation
}

SmallShell::~SmallShell() {
    // TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command* SmallShell::CreateCommand(const char* cmd_line) {
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    std::vector<std::pair<std::string, std::string>>* aliases =
        SmallShell::getInstance().getAliases();
    for (const auto& p : *aliases) {
        if (firstWord == p.first) {
            cmd_s = p.second + cmd_s.substr(firstWord.length());
            firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
            break;
        }
    }

    if (firstWord.compare("chprompt") == 0) return new ChpromptCommand(cmd_s.c_str());
    else if (firstWord.compare("quit") == 0) return new QuitCommand(cmd_s.c_str(), &jobs);
    else if (firstWord.compare("kill") == 0) return new KillCommand(cmd_s.c_str(), &jobs);
    else if (firstWord.compare("alias") == 0) return new AliasCommand(cmd_s.c_str());
    else if (firstWord.compare("showpid") == 0) return new ShowPidCommand(cmd_s.c_str());
    else if (firstWord.compare("pwd") == 0) return new PwdCommand(cmd_s.c_str());
    else if (firstWord.compare("cd") == 0) return new CdCommand(cmd_s.c_str());
    else if (firstWord.compare("jobs") == 0) return new JobsCommand(cmd_s.c_str(), &jobs);
    return nullptr;
}

void SmallShell::executeCommand(const char* cmd_line) {
    Command* cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) {
        cmd->execute();
        delete cmd;
    }
}

Command::Command(const char* cmd_line) : cmdLine(cmd_line) {
    this->pid = getpid();
}

Command::~Command() {
}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line) {
}

std::string SmallShell::getPrompt() const {
    return prompt;
}

void SmallShell::setPrompt(const std::string& newPrompt) {
    this->prompt = newPrompt;
}

std::string SmallShell::getLastDir() const {
    return this->lastDir;
}

void SmallShell::setLastDir(const std::string& newLastDir) {
    this->lastDir = newLastDir;
}

ChpromptCommand::ChpromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    if (num_of_args > 1) {
        this->name = args[1];
    } else this->name = "smash";

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void ChpromptCommand::execute() {
    SmallShell::getInstance().setPrompt(this->name);
}

QuitCommand::QuitCommand(const char* cmd_line, JobsList* j) : BuiltInCommand(cmd_line) {
    jobs = j;
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    if (num_of_args > 1) {
        if (std::string(args[1]) == "kill") kill_active = true;
    } else kill_active = false;

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {
    this->jobs = jobs;
    this->valid = true;
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    if (num_of_args != 3) {
        valid = false;
    } else {
        try {
            if (args[1][0] != '-') {
                valid = false;
            } else {
                this->signum = std::stoi(std::string(args[1] + 1));
                this->jobId = std::stoi(std::string(args[2]));
                if (std::stoi(std::string(args[1] + 1)) < 0 ||
                    std::stoi(std::string(args[1] + 1)) > 31)
                    valid = false;
                else valid = true;
            }
        } catch (...) {
            valid = false;
        }
    }
    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void KillCommand::execute() {
    if (!valid) {
        std::cerr << "smash error: kill: invalid arguments\n";
        return;
    }
    JobsList::JobEntry* j = jobs->getJobById(this->jobId);
    if (j == nullptr) {
        std::cerr << "smash error: kill: job-id " << this->jobId << " does not exist\n";
        return;
    }
    int processId = j->getProcessId();
    if (kill(processId, signum) == -1) {
        perror("smash error: kill failed");
        return;
    }
    std::cout << "signal number " << this->signum << " was sent to pid " << processId << "\n";
}

void QuitCommand::execute() {
    if (kill_active) {
        jobs->killAllJobs();
    }
    exit(0);
}

JobsList::JobsList() : max_job_id(0) {
}

JobsList::~JobsList() {
}

void JobsList::killAllJobs() {
    std::cout << "sending SIGKILL signal to " <<
        SmallShell::getInstance().getJobslist()->jobs_map.size() << " jobs:\n";
    for (auto const& pair : jobs_map) {
        std::cout << pair.first << ": " << pair.second.getCmd_line() << "\n";
        kill(pair.second.getProcessId(), SIGKILL);
    }
}

JobsList::JobEntry* JobsList::getJobById(int jobId) {
    auto it = jobs_map.find(jobId);
    if (it != jobs_map.end()) return &(it->second);
    return nullptr;
}

AliasCommand::AliasCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    this->is_print_only = false;
    this->error_type = 0;
    std::string cmd_str = _trim(cmd_line);
    if (cmd_str == "alias") {
        is_print_only = true;
        return;
    }

    std::regex given_regex(R"(^alias [a-zA-Z0-9_]+='[^']*'$)");
    if (!std::regex_match(cmd_str, given_regex)) {
        error_type = 1;
        return;
    }

    size_t equal_indx = cmd_str.find('=');
    this->name = cmd_str.substr(6, equal_indx - 6);
    size_t quote_indx = equal_indx + 2;
    size_t command_len = cmd_str.length() - quote_indx - 1;
    this->command = cmd_str.substr(quote_indx, command_len);
}

void AliasCommand::execute() {
    if (error_type == 1) {
        std::cerr << "smash error: alias: invalid alias format\n";
        return;
    }
    std::vector<std::pair<std::string, std::string>>* aliases =
        SmallShell::getInstance().getAliases();
    if (is_print_only) {
        for (const auto& p : *aliases) {
            std::cout << p.first << "='" << p.second << "'\n";
        }
        return;
    }
    std::vector<std::string> reserved_keywords = {"chprompt", "showpid", "pwd", "cd", "jobs", "fg",
        "quit", "kill", "alias", "unalias", "unsetenv", "sysinfo"
    };
    for (const auto& key : reserved_keywords) {
        if (this->name == key) {
            std::cerr << "smash error: alias: " << this->name <<
                " already exists or is a reserved command\n";
            return;
        }
    }
    for (const auto& p : *aliases) {
        if (p.first == this->name) {
            std::cerr << "smash error: alias: " << this->name <<
                " already exists or is a reserved command\n";
            return;
        }
    }
    aliases->push_back(std::make_pair(this->name, this->command));
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << getpid() << endl;
}

ShowPidCommand::ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
}


PwdCommand::PwdCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
}

void PwdCommand::execute() {
    char path[PATH_MAX];
    getcwd(path, PATH_MAX);
    cout << path << endl;
}

CdCommand::CdCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    this->numOfArgs = num_of_args - 1;
    if (this->numOfArgs == 0) {
        char curDir[PATH_MAX];
        getcwd(curDir, PATH_MAX);
        this->newDir = curDir;
    } else {
        this->newDir = args[1];
    }

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void CdCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();

    char curDir[PATH_MAX];
    getcwd(curDir, PATH_MAX);

    if (numOfArgs == 0) {
        return;
    }

    if (this->numOfArgs > 1) {
        cout << "smash error: cd: too many arguments" << endl;
        return;
    }

    if (newDir == "-") {
        if (smash.getLastDir() == "") {
            cout << "smash error: cd: OLDPWD not set" << endl;
            return;
        }

        newDir = smash.getLastDir();
    }

    chdir(newDir.c_str());
    smash.setLastDir(curDir);
}

JobsCommand::JobsCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line) {
    this->jobs = jobs;
}

void JobsCommand::execute() {
    jobs->removeFinishedJobs();
    jobs->printJobsList();
}

void JobsList::printJobsList() {
    for (const auto& [key, value] : this->jobs_map) {
        cout << "[" << value.getJobId() << "]" << value.getCmd_line() << endl;
    }
}

void JobsList::addJob(Command* cmd, bool isStopped) {
    ++max_job_id;
    jobs_map[max_job_id] = JobEntry(max_job_id, cmd->getPid(), cmd->getCmdLine(), isStopped);
}

std::string Command::getCmdLine() const {
    return this->cmdLine;
}

pid_t Command::getPid() const {
    return this->pid;
}


