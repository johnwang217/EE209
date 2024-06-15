#ifndef _ISH_H_
#define _ISH_H_

struct Command {
  char **arguments;
  char *redin;
  char *redout;
  char **pipes;
  int arg_index;
  int pip_index;
};

void SIGQUIT_Handler(int signum);
void exitTimer(int signum);
void SIGALARM_Handler(int signum);

#endif /* _ISH_H_ */