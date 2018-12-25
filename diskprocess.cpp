#include<iostream>
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
#include <cstring>
#define NoOfSlots 10
using namespace std;
struct msgbuff
{
	long mtype; // must be long
	char mtext[64];
};
int prev_clk=-1,clk=-1;
int freeSlots=10;
key_t upMsgqId,downMsgqId;
bool slotfull[NoOfSlots];
char Slots [NoOfSlots][64];


void IncClk(int signum) // Handler for SIGUSR2
{
	prev_clk=clk;
	clk++;
	cout<<"MY clk got incremented"<<clk<<endl;
}

//send via up queue
void SendMsg(int signum) // Handler for SIGUSR1
{
	int send_val;

	struct msgbuff message;

	message.mtype = getpid();    ///

	string msg ="";
	for (int i=0;i<NoOfSlots;i++)
		if (slotfull[i])
			msg+= "1";
		else
			msg+="0";
	strcpy(message.mtext, msg.c_str());
	// busy wait until msg sent // should i ?
	cout<<" sending response to kernel "<<endl;
	send_val = msgsnd(upMsgqId, &message, sizeof(message.mtext), IPC_NOWAIT);

	if(send_val == -1)
		perror("Error in send");
	else
	cout<<" response sent FreeSlots  :"<<message.mtext<<endl;
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
	strcpy(message.mtext,"D");
	
	// busy wait until msg sent 
	send_val = msgsnd(upMsgqId, &message, sizeof(message.mtext), IPC_NOWAIT);
	printf("Sending MY Id to Kernel %d\n",myId);
	cout<<message.mtext<<endl;
	if(send_val == -1)
		perror("Errror in send");
}
void Add(char * msg)
{
	// searches for an empty slot and adds the msg in it
	// takes 3 secs
	int i=0;
	while (slotfull[i] && i<NoOfSlots) // false :empty , true :full
		i++;
	if (i!=NoOfSlots) 
	{
		strcpy(Slots[i],msg);
		freeSlots-=1;
		slotfull[i]=true;
	}
}
void Delete(int slotno)
{
	// deletes the msg in the slot no given
	//takes 1 sec
	if (slotno<NoOfSlots && slotfull[slotno])
	{
		//delete the char *
		memset(Slots[slotno], '\0', sizeof(Slots[slotno]));
		freeSlots+=1;
		slotfull[slotno]=false;
	}
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
		perror("Errrrrror in receive");
	//if(rec_val == -1)
	else
	{
		char *str;
		strcpy(str,message.mtext);
		cout<<" recieved "<<message.mtext<<endl;
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
	
	upMsgqId = msgget(99, IPC_CREAT|0644);//99 for up // dummy values 
	downMsgqId = msgget(100, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
	cout<<"Disk Up Queue ID "<<upMsgqId<<endl;
	cout<<"Disk down Queue ID "<<downMsgqId<<endl;
	sendMyID();
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
