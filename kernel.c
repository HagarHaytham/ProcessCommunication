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
#include<vector>
struct msgbuff
{
   long mtype; // must be long
   char mtext[64];
};


int main()
{
	vector <msgbuff> log;
	upMsgqId = msgget(99, IPC_CREAT|0644);//99 for up // dummy values 
	downMsgqId = msgget(100, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
	int groub_id;
	getpgid(groub_id);	
	int clk=0;
	while(1)
	{
	killpg(groub_id,SIGUSR2);
	struct msgbuff message;

	recieve = msgrcv(upMsgqId, &message, sizeof(message.mtext),0, !IPC_NOWAIT);  
	if(recieve !=-1 )
	{


		
	}

	clk++;
	}
	return 0;
}