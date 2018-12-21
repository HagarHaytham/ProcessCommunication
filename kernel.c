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
#include <vector>
#include <string>
const int disk_type =0;
const int process_type=1;
const int disk_up=99;
const int disk_down=100;
const int process_up=199;
const int process_down=200;
int disk_id;
int process_counter=0;
vector <string> log;
vector <int> process_list;
int disk_up_queue = msgget(disk_up, IPC_CREAT|0644);//99 for up // dummy values 
int disk_down_queue = msgget(disk_down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
int process_up_queue = msgget(process_up, IPC_CREAT|0644);//99 for up // dummy values 
int process_down_queue = msgget(process_down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
struct msgbuff
{
   long mtype; // must be long
   char request_type;
   char mtext[64];
};

void initialize(disk_up_queue,process_up_queue)
{
	process_Counter=0;
	struct msgbuff first_message;
        recieve = msgrcv(disk_up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down
      	disk_id=first_message.mtype;
	recieve = msgrcv(process_up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT); // receive on up , send on down   
	process_list[process_counter++]=first_message.mtype;
	
}
int disk_status()
{
struct msgbuff message;
recieve = msgrcv(disk_up_queue, &message, sizeof(message.mtext),0, !IPC_NOWAIT); 
if(recieve != -1 && message.mtype=disk_id )
return (int)message.mtext;
return -1;
}
int process_request(struct msgbuff message)
{
if(message.request_type == 'I')
{
	process_list[process_counter++]=(int)message.mtype;

}
else
{ 
	struct msgbuff kernel_response;
	int latency=0;
	killpg(disk_id,SIGUSR1);
	int disk_response=disk_status();

	if( disk_response!=-1 )
	{

	if(message.request_type == 'D' && disk_response >0)
	{
		message.mtext=1;
		letency=1;
	}
	else 	message.mtext=3;
	if(message.request_type == 'A' && disk_response <10)
	{
		message.mtext=0;
		letency=3;
	}
	else 	message.mtext=2;
	kernel_response.mtype=message.mtype;
	send = msgsnd(upMsgqId, &message, sizeof(message.mtext), IPC_NOWAIT);
	}

}
}



int main()
{

	initialize(disk_up_queue,process_up_queue);
	
	int clk=0;
	while(1)
	{
	for(int i=0;i<process_id.size();i++)
	killpg(process_list[i],SIGUSR2);
	
	struct msgbuff message;
	int latency=0;
	recieve = msgrcv(process_up_queue, &message, sizeof(message.mtext),0, IPC_NOWAIT);  
	if(recieve !=-1 )
	{
	log.push_back(message.request_type+" "+message.mtext);
	
	latency=process_request(message);
	}
	if(latency !=0 )
		clk+=latency;
	else
		clk++;
	}
	return 0;
}
