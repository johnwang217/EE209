#ifndef _ISH_H_
#define _ISH_H_

#include "util.h"
#include "command.h"

#define EXEC_FAIL -1
#define EXEC_SUCC 0

void SIGQUIT_Handler(int signum);
void exitTimer(int signum);
void SIGALARM_Handler(int signum);
int execBuiltin(enum BuiltinType btype, struct Command *input);

#endif /* _ISH_H_ */