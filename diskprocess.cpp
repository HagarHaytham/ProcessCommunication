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

	// itoa is not standard function !! can't use it under linux
	//itoa(freeSlots, str, 10);// base 10 decimal

	// message txt is no of free slots
	int length = snprintf( NULL, 0, "%d", freeSlots );
	char* str = new char [length+1];
	snprintf( str, length + 1, "%d", freeSlots );
	strcpy(message.mtext, str);

	// busy wait until msg sent // should i ?
	send_val = msgsnd(upMsgqId, &message, sizeof(message.mtext), !IPC_NOWAIT);

	if(send_val == -1)
		perror("Error in send");

}

char  * RemoveFirstLetter(char *msg)
{
	char *p = new char[ sizeof(*p) * strlen(msg) ];
	int i;
	for(i=0; i<strlen(msg); i++)
	{
		p[i]=msg[i+1];
	}

	return p;
}

void sendMyID()
{
	int myId = getpid();
	int send_val;

	struct msgbuff message;

	message.mtype = myId;
	strcpy(message.mtext,"dp");///// check that with reham
	
	// busy wait until msg sent 
	send_val = msgsnd(upMsgqId, &message, sizeof(message.mtext), !IPC_NOWAIT);
	printf("Sending MY Id to Kernel %d\n",myId);
	if(send_val == -1)
		perror("Errror in send");
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
	if (i==NoOfSlots) 
		diskStatus=2;// failed to add
	else
	{
		strcpy(Slots[i],msg);
		freeSlots-=1;
		slotfull[i]=true;
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
		memset(Slots[slotno], '\0', sizeof(Slots[slotno]));
		freeSlots+=1;
		slotfull[slotno]=false;
		diskStatus=1; // successful delete
	}
	else
		diskStatus=3; // failed to delete
}

// recieve via down queue
void RecieveMsg()
{
	int rec_val;
	struct msgbuff message;

	//rec_val=1;// ay klam delwa2ty
	/* receive all types of messages */
	rec_val = msgrcv(downMsgqId, &message, sizeof(message.mtext),0, !IPC_NOWAIT);  

	if(rec_val == -1)
		perror("Error in receive");
	else
	{
		char *str;
		strcpy(str,message.mtext);
		if (str[0]=='A')
		{
			// ashel awel 7rf
			//memcpy(dest, src+1,sizeof(src));
			str = RemoveFirstLetter(str);
			Add(str);
		}
		else 
		{
			// ashel awel 7rf w a7wel l int
			str = RemoveFirstLetter(str);
			int x;
			sscanf( str, "%d", &x);
			Delete(x);
		}
	}
}


int main()
{
	printf("Begin Of Disk MAIN \n");
	signal(SIGUSR2,IncClk);	
	signal(SIGUSR1,SendMsg);
	memset(slotfull ,0, sizeof(slotfull[0]) * NoOfSlots); // all slots are empty at the begining
	// first msg sent to kernel is my id to let it communicate with me through signals
	sendMyID();
	upMsgqId = msgget(99, IPC_CREAT|0644);//99 for up // dummy values 
	downMsgqId = msgget(100, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
	if(upMsgqId == -1 ||downMsgqId == -1 ){	
		perror("Error in create");
		exit(-1);}
	// so for syncronization we have to work with the same clk given 
	while (clk==prev_clk)// -1 at first , the disk process shouldn't start now
		continue;

	while(1)
		RecieveMsg();

	return 0;
}
