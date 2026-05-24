#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

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

    if (firstWord.compare("chprompt") == 0) return new ChpromptCommand(cmd_line);
    else if (firstWord.compare("quit") == 0) return new QuitCommand(cmd_line, &jobs);
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