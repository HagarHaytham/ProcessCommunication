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
const int disk_type =0;
const int process_type=1;
const int up=99;
const int down=100;
struct msgbuff
{
   long mtype; // must be long
   char mtext[64];
};


int main()
{
	vector <msgbuff> log;
	upMsgqId = msgget(up, IPC_CREAT|0644);//99 for up // dummy values 
	downMsgqId = msgget(down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
	
	int disk_id;
	int process_id;
	struct msgbuff first_message;
	for(int i =0 ;i<2 ;i++)
	{
	
	recieve = msgrcv(upMsgqId, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down

	    if(first_message.mtype == disk_type)
	    	disk_id=first_message.mtext;
   	    else if(first_message.mtype == process_type)
	    	process_id=first_message.mtext;

	}
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
