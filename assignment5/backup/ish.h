#ifndef _ISH_H_
#define _ISH_H_

#include "util.h"
#include "command.h"

void SIGQUIT_Handler(int signum);
void exitTimer(int signum);
void SIGALARM_Handler(int signum);
int execBuiltin(struct Command *c, enum BuiltinType btype);
int execCommand(struct Command *c);
void loadPipetoArg(struct Command *c);

#endif /* _ISH_H_ */