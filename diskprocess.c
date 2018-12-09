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
int diskStatus =-1;
int freeSlots=10;
key_t upMsgqId,downMsgqId;

void IncClk(int signum) // Handler for SIGUSR2
{
	clk++;
}

//send via up queue
void SendMsg(int signum) // Handler for SIGUSR1
{
	int send_val;

	struct msgbuff message;

	message.mtype = diskStatus;

	char *str;
	// itoa is not standard function !! can't use it under linux
	//itoa(freeSlots, str, 10);// base 10 decimal
	strcpy(message.mtext, str);

	// busy wait until msg sent // should i ?
	send_val = msgsnd(upMsgqId, &message, sizeof(message.mtext), !IPC_NOWAIT);

	if(send_val == -1)
		perror("Errror in send");

}

// recieve via down queue
void RecieveMsg()
{
	int rec_val;
	struct msgbuff message;

	rec_val=1;// ay klam delwa2ty
	/* receive all types of messages */
	rec_val = msgrcv(downMsgqId, &message, sizeof(message.mtext),0, !IPC_NOWAIT);  

	if(rec_val == -1)
		perror("Error in receive");
	//else
		//printf("\nMessage received: %s\n", message.mtext);

}

int main()
{
	signal(SIGUSR2,IncClk);
	signal(SIGUSR1,SendMsg);
	upMsgqId = msgget(99, IPC_CREAT|0644);//99 for up // dummy values 
	downMsgqId = msgget(100, IPC_CREAT|0644);// 100 for down
	if(upMsgqId == -1 ||downMsgqId == -1 )
	{	
		perror("Error in create");
		exit(-1);
	}
	char Slots [10][64];




	return 0;
}