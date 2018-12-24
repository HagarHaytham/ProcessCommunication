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
using namespace std;
const int disk_type =0;
const int process_type=1;
const int disk_up=99;
const int disk_down=100;
const int process_up=199;
const int process_down=200;
int disk_id;
int process_counter=0;
vector <string> message_log;
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

void initialize(int disk_up_queue,int process_up_queue)
{
	printf("didn't get The ID From DISK yet");
	struct msgbuff first_message;
	int recieve = msgrcv(disk_up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down
	disk_id=first_message.mtype;
	printf("GOT The ID From DISK , READY FOR comm %d",disk_id);
	recieve = msgrcv(process_up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT); // receive on up , send on down   
	process_list.push_back(first_message.mtype);

}
int disk_status()
{
	struct msgbuff message;
	int recieve = msgrcv(disk_up_queue, &message, sizeof(message.mtext),0, !IPC_NOWAIT); 
	if(recieve != -1 && message.mtype==disk_id )
		return message.request_type;
	return -1;
}
int process_request(struct msgbuff message)
{
	int latency=0;
	if(message.request_type == 'I')
	{
		process_list[process_counter++]=(int)message.mtype;

	}
	else
	{ 
		struct msgbuff kernel_response;

		killpg(disk_id,SIGUSR1);
		int disk_response=disk_status();

		if( disk_response!=-1 )
		{

			if(message.request_type == 'D' && disk_response >0)
			{
				kernel_response.request_type=1;
				int send = msgsnd(disk_down_queue, &kernel_response, sizeof(message.mtext), IPC_NOWAIT);
				latency=1;
			}
			else 	kernel_response.request_type=3;
			if(message.request_type == 'A' && disk_response <10)
			{
				kernel_response.request_type=0;
				int send = msgsnd(disk_down_queue, &kernel_response, sizeof(message.mtext), IPC_NOWAIT);
				latency=3;
			}
			else 	kernel_response.request_type=2;
			kernel_response.mtype=message.mtype;
			int send = msgsnd(process_down_queue, &kernel_response, sizeof(message.mtext), IPC_NOWAIT);
		}

	}
	return 	latency;
}



int main()
{
	printf("Begin Of Kernel MAIN \n");
	initialize(disk_up_queue,process_up_queue);

	int clk=0;
	while(1)
	{
		for(int i=0;i<process_list.size();i++)
			killpg(process_list[i],SIGUSR2);

		struct msgbuff message;
		int latency=0;
		int recieve = msgrcv(process_up_queue, &message, sizeof(message.mtext),0, IPC_NOWAIT);  
		if(recieve !=-1 )
		{
			message_log.push_back(message.request_type+" "+(string)message.mtext);

			latency=process_request(message);
		}
		if(latency !=0 )
			clk+=latency;
		else
			clk++;
	}
	return 0;
}
