#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <regex>
#include <fstream>
#include <sys/utsname.h> 
#include <sys/sysinfo.h> 
#include <time.h>
extern char **__environ;

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

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;
    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
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
Command *SmallShell::CreateCommand(const char *cmd_line) {
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    std::vector<std::pair<std::string, std::string>>* aliases = 
        SmallShell::getInstance().getAliases();
    for (const auto& p : *aliases)
    {
        if (firstWord == p.first)
        {
            cmd_s = p.second + cmd_s.substr(firstWord.length());
            firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
            break;
        }
    }

    if (firstWord.compare("chprompt") == 0) return new ChpromptCommand(cmd_line);
    else if (firstWord.compare("quit") == 0) return new QuitCommand(cmd_line, &jobs);
    else if (firstWord.compare("kill") == 0) return new KillCommand(cmd_line, &jobs);
    else if (firstWord.compare("alias") == 0) return new AliasCommand(cmd_line);
    else if (firstWord.compare("unalias") == 0) return new UnAliasCommand(cmd_line);
    else if (firstWord.compare("unsetenv") == 0) return new UnSetEnvCommand(cmd_line);
    else if(firstWord.compare("sysinfo") == 0) return new SysInfoCommand(cmd_line);
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command* cmd = CreateCommand(cmd_line);
    if (cmd != nullptr) 
    {
        cmd->execute();
        delete cmd;
    }
}
Command::Command(const char* cmd_line)
{
    
}

Command::~Command()
{

}

BuiltInCommand::BuiltInCommand(const char* cmd_line) : Command(cmd_line)
{

}

ChpromptCommand::ChpromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    if (num_of_args > 1)
    {
        this->name = args[1];
    }
    else this->name = "smash";

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void ChpromptCommand::execute(){

    SmallShell::getInstance().setPrompt(this->name);
}

QuitCommand::QuitCommand(const char* cmd_line, JobsList *j) : BuiltInCommand(cmd_line){
    jobs = j;
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    if (num_of_args > 1)
    {
        if (std::string(args[1]) == "kill") kill_active = true;
    }
    else kill_active = false;

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line)
{
    this->jobs = jobs;
    this->valid = true;
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);
    if (num_of_args != 3)
    {
        valid = false;
    }
    else
    {
        try
        {
            if (args[1][0] != '-') 
            {
                valid = false;
            }
            else
            {
                this->signum = std::stoi(std::string(args[1]+1));
                this->jobId = std::stoi(std::string(args[2]));
                if (std::stoi(std::string(args[1]+1)) < 0 || 
                std::stoi(std::string(args[1]+1)) > 31) valid = false;
                else valid = true;
            }
        }
        catch(...)
        {
            valid = false;
        }
    }
    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void KillCommand::execute()
{
    if (!valid) 
    {
        std::cerr << "smash error: kill: invalid arguments\n";
        return;
    }
    JobsList::JobEntry* j = jobs->getJobById(this->jobId);
    if (j == nullptr)
    {
        std::cerr << "smash error: kill: job-id " << this->jobId << " does not exist\n";
        return;
    }
    int processId = j->getProcessId();
    if (kill(processId, signum) == -1) 
    {
        perror("smash error: kill failed");
        return;
    }
    std::cout << "signal number " << this->signum << " was sent to pid " << processId << "\n";

}

void QuitCommand::execute(){
    if (kill_active)
    {
        jobs->killAllJobs();
    }
    exit(0);
}

JobsList::JobsList() : max_job_id(0) {}

JobsList::~JobsList() {}

void JobsList::killAllJobs() {
    std::cout << "sending SIGKILL signal to " << 
    SmallShell::getInstance().getJobslist()->jobs_map.size() << " jobs:\n";
    for (auto const& pair : jobs_map)
    {
        std::cout << pair.first << ": " << pair.second.getCmd_line() << "\n";
        kill(pair.second.getProcessId(), SIGKILL);
    }
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
    auto it = jobs_map.find(jobId);
    if (it != jobs_map.end()) return &(it->second);
    return nullptr;
}

AliasCommand::AliasCommand(const char *cmd_line) : BuiltInCommand(cmd_line) 
{
    this->is_print_only = false;
    this->error_type = 0;
    std::string cmd_str = _trim(cmd_line);
    if (cmd_str == "alias") 
    {
        is_print_only = true;
        return; 
    }

    std::regex given_regex(R"(^alias [a-zA-Z0-9_]+='[^']*'$)");
    if (!std::regex_match(cmd_str, given_regex))
    {
        error_type = 1;
        return;
    }

    size_t equal_indx = cmd_str.find('=');
    this->name = cmd_str.substr(6, equal_indx - 6);
    size_t quote_indx = equal_indx + 2;
    size_t command_len = cmd_str.length() - quote_indx - 1;
    this->command = cmd_str.substr(quote_indx, command_len);
}

void AliasCommand::execute()
{
    if (error_type == 1)
    {
        std::cerr << "smash error: alias: invalid alias format\n";
        return;
    }
    std::vector<std::pair<std::string, std::string>>* aliases = 
        SmallShell::getInstance().getAliases();
    if (is_print_only)
    {
        for (const auto& p : *aliases)
        {
            std::cout << p.first << "='" << p.second << "'\n";
        }
        return;
    }
    std::vector<std::string> reserved_keywords = {"chprompt", "showpid", "pwd", "cd", "jobs"
        , "fg", "quit", "kill", "alias", "unalias", "unsetenv", "sysinfo"};
    for (const auto& key : reserved_keywords)
    {
        if (this->name == key) 
        {
            std::cerr << "smash error: alias: " << this->name << " already exists or is a reserved command\n";
            return;
        }
    }
    for (const auto& p : *aliases)
    {
        if (p.first == this->name)
        {
            std::cerr << "smash error: alias: " << this->name << " already exists or is a reserved command\n";
            return;
        }
    }
    aliases->push_back(std::make_pair(this->name, this->command));
}

UnAliasCommand::UnAliasCommand(const char* cmd_line) : BuiltInCommand(cmd_line)
{
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);

    if (num_of_args <= 1) this->not_enough_args = true;

    for (int i = 1; i < num_of_args; ++i) this->remove_aliases.push_back(args[i]);

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void UnAliasCommand::execute()
{
    if (not_enough_args) 
    {
        std::cerr << "smash error: unalias: not enough arguments\n";
        return;
    }
    std::vector<std::pair<std::string, std::string>>* aliases = 
        SmallShell::getInstance().getAliases();
    bool is_found = false;
    
    for (auto& name : remove_aliases)
    {
        for (auto it = aliases->begin(); it != aliases->end(); ++it)
        {
            if (name == it->first)
            {
                aliases->erase(it);
                is_found = true;
                break;
            }
        }
        if (!is_found)
        {
            std::cerr << "smash error: unalias: " << name << " alias does not exist\n";
            return;
        }
        is_found = false;
    }
}

UnSetEnvCommand::UnSetEnvCommand(const char* cmd_line) : BuiltInCommand(cmd_line)
{
    char* args[COMMAND_MAX_ARGS];
    int num_of_args = _parseCommandLine(cmd_line, args);

    if (num_of_args <= 1) this->not_enough_args = true;

    for (int i = 1; i < num_of_args; ++i) this->remove_envvar.push_back(args[i]);

    for (int i = 0; i < num_of_args; ++i) free(args[i]);
}

void UnSetEnvCommand::execute()
{
    if (not_enough_args) 
    {
        std::cerr << "smash error: unsetenv: not enough arguments\n";
        return;
    }

    pid_t pid = getpid();
    std::string path = "/proc/" + std::to_string(pid) + "/environ";
    bool is_found = false;
    
    for (auto& name : remove_envvar)
    {
        std::string search_key = name + "=";
        std::ifstream env_file(path);
        if (env_file.is_open())
        {
            std::string entry;
            while (std::getline(env_file, entry, '\0'))
            {
                if (entry.find(search_key) == 0)
                {
                    is_found = true;
                    break;
                }
            }
        }

        if (!is_found)
        {
            std::cerr << "smash error: unsetenv: " << name << " does not exist\n";
            return;
        }

        for (int i = 0; __environ[i] != nullptr; ++i)
        {
            std::string current_env(__environ[i]);
            if (current_env.find(search_key) == 0)
            {
                for (int j = i; __environ[j] != nullptr; ++j) __environ[j] = __environ[j+1];
                break;
            }
        }
        is_found = false;
    }
}

SysInfoCommand::SysInfoCommand(const char* cmd_line) : BuiltInCommand(cmd_line) {}

void SysInfoCommand::execute()
{
    struct utsname name_data;
    struct sysinfo sys_info;

    if (uname(&name_data) == -1) return;
    if (sysinfo(&sys_info) == -1) return;

    time_t current_time = time(NULL);
    time_t boot_time = current_time - sys_info.uptime;
    struct tm* boot_tm = localtime(&boot_time);

    std::cout << "System: " << name_data.sysname << "\n";
    std::cout << "Hostname: " << name_data.nodename << "\n";
    std::cout << "Kernel: " << name_data.release << "\n";
    std::cout << "Architecture: " << name_data.machine << "\n";
    std::cout << "Boot Time: " << std::put_time(boot_tm, "%Y-%m-%d %H:%M:%S") << "\n";
}