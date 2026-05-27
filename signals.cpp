#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    std::cout << "smash: got ctrl-C\n";

    int processId = SmallShell::getInstance().getFg_pid();
    if (processId != -1)
    {
        if (kill(processId, SIGKILL) == -1) {
            perror("smash error: kill failed");
            return;
        }
    }

    std::cout << "smash: process" << processId << "was killed\n";
    SmallShell::getInstance().setFg_pid(-1);
}
