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
#include <stdbool.h> 
#define NoOfSlots 10

struct msgbuff
{
	long mtype; // must be long
	char mtext[64];
};
//const int NoOfSlots=10;// this is not really a const in c ,, can't use it 
int prev_clk=-1,clk=-1;
int diskStatus =6;// disk status 
// succ add 0
// succ del 1
// add failed 2
//deletion failed 3
// Adding takes 3 clks?
//deletion takes 1 clk?
// adding 4
// deleting 5
// nothing 6
int freeSlots=10;
key_t upMsgqId,downMsgqId;
bool slotfull[NoOfSlots];
char Slots [NoOfSlots][64];


void IncClk(int signum) // Handler for SIGUSR2
{
	prev_clk=clk;
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

void Add(char * msg)
{
	// searches for an empty slot and adds the msg in it
	// takes 3 secs
	int begin =clk; // begin now
	diskStatus =4;//adding
	int i=0;
	while (slotfull[i] && i<NoOfSlots) // false :empty , true :full
		i++;
	while (begin +3 >clk)
		continue;// just wait 3 seconds to add
	if (i==NoOfSlots) // aw el msg akbr mn 64 ??????????
		diskStatus=2;// failed to add
	else
	{
		strcpy(Slots[i],msg);
		diskStatus=0;// successful add
	}


}
void Delete(int slotno)
{
	// deletes the msg in the slot no given
	//takes 1 sec
	int begin = clk;
	diskStatus= 5;//deleting
	while (begin +1 >clk)
		continue;// just wait 1 seconds to delete
	if (slotno<NoOfSlots && slotfull[slotno])
	{
		//delete the char *
		diskStatus=1; // successful delete
	}
	else
		diskStatus=3; // failed to delete
}

int main()
{
	signal(SIGUSR2,IncClk);
	signal(SIGUSR1,SendMsg);
	memset(slotfull ,0, sizeof(slotfull[0]) * NoOfSlots); // all slots are empty at the begining
	// kernel should wait until both processes are running 
	// so an idea to handle this is to let te disk process create the msg queues 
	//and the kernel waits until they are created , same with the Process process
	// we can also send a signal maybe ?
	upMsgqId = msgget(99, IPC_CREAT|0644);//99 for up // dummy values 
	downMsgqId = msgget(100, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
	if(upMsgqId == -1 ||downMsgqId == -1 )
	{	
		perror("Error in create");
		exit(-1);
	}
	prev_clk=-1;
	clk=-1;
	// so for syncronization we have to work with the same clk given 
	while (clk==prev_clk)// -1 at first , the disk process shouldn't start now
		continue;



	return 0;
}