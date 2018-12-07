#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

struct msgbuff
{
   long mtype; // must be long
   char mtext[64];
};

int clk =0;
void IncClk(int signum)
{
	clk++;
}

int main()
{
	signal(SIGUSR2,IncClk);
	
	
	return 0;
}