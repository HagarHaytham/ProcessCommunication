#include<iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <vector>
#include <string>
using namespace std;
using namespace std::chrono;

struct msgbuff
{

   long mtype; // must be long
   char mtext[64];
};

const int disk_type =0;
const int process_type=1;
const int disk_up=99;
const int disk_down=100;
const int process_up=199;
const int process_down=200;
int disk_id;
int process_counter=0;
vector <msgbuff> message_log;
vector <msgbuff> disk_log;
vector <int> response_log;
vector <int> disk_status_log;
vector <long> process_list;
int disk_up_queue = msgget(disk_up, IPC_CREAT|0644);//99 for up // dummy values 
int disk_down_queue = msgget(disk_down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
int process_up_queue = msgget(process_up, IPC_CREAT|0644);//99 for up // dummy values 
int process_down_queue = msgget(process_down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys

int get_time()
{
	seconds s = duration_cast< seconds >(system_clock::now().time_since_epoch());
	int time= s.count();
	return time;
}


void initialize(int disk_up_queue,int process_up_queue)
{
	printf("didn't get The ID From DISK yet");
	struct msgbuff first_message;
	cout<<"up queue id = "<<disk_up_queue<<endl;
	int recieve = msgrcv(disk_up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down
	disk_id=first_message.mtype;
	cout<<"GOT The ID From DISK , READY FOR comm "<<disk_id<<endl;
	cout<<"up queue id = "<<process_up_queue<<endl;
	recieve = msgrcv(process_up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT); // receive on up , send on down   
	cout<<"GOT The ID From process , READY FOR comm "<<disk_id<<endl;
	process_list.push_back(first_message.mtype);

}
int disk_status()
{
	struct msgbuff message;
	int recieve = msgrcv(disk_up_queue, &message, sizeof(message.mtext),0, !IPC_NOWAIT); 
	if(recieve != -1 && message.mtype==disk_id )
	{
		return message.mtext[0];
		disk_status_log.push_back(message.mtext[0]);
	}
	return -1;
}
int process_request(struct msgbuff message)
{
	int latency=0;

		struct msgbuff kernel_response;

		killpg(disk_id,SIGUSR1);
		int disk_response=disk_status();
		cout<<"disk_response "<<disk_response<<endl;
		if( disk_response!=-1 )
		{

			if(message.mtext[0] == 'D' && disk_response >0)
			{
				kernel_response.mtext[0]=1;
				
				int send = msgsnd(disk_down_queue, &message, sizeof(message.mtext), IPC_NOWAIT);
				disk_log.push_back(message);
				latency=1;
			}
			else 	kernel_response.mtext[0]=3;
			if(message.mtext[0] == 'A' && disk_response <10)
			{
				kernel_response.mtext[0]=0;
				int send = msgsnd(disk_down_queue, &message, sizeof(message.mtext), IPC_NOWAIT);
				disk_log.push_back(message);
				latency=3;
			}
			else 	kernel_response.mtext[0]=2;
			kernel_response.mtype=message.mtype;
			int send = msgsnd(process_down_queue, &kernel_response, sizeof(message.mtext), IPC_NOWAIT);
			response_log.push_back(kernel_response.mtext[0]);
		}


	return 	latency;

}



int main()
{

	printf("Begin Of Kernel MAIN \n");
	initialize(disk_up_queue,process_up_queue);
	
	int clk=-1;
	int current_time=0;
	int latency=0;
	struct msgbuff message;
	int prev_time=get_time();
	while(1)
{
	current_time=get_time();
	int recieve = msgrcv(process_up_queue, &message, sizeof(message.mtext),0, IPC_NOWAIT);  
	if(recieve !=-1 )
	{
	cout<<" recieved message "<<message.mtext<<endl;
	process_list.push_back(message.mtype);
	message_log.push_back(message);
	}
	
	if(current_time-prev_time == 1)
	{
	
	prev_time = current_time;
	clk++;
	if(latency > 0)
	latency--;
	for(int i=0;i<process_list.size();i++)
	killpg(process_list[i],SIGUSR2);
	killpg(disk_id,SIGUSR2);
	for(int i=0;i<message_log.size();i++)
		{
		if(latency ==0)
		latency=process_request(message);
		}
	}	
}


	return 0;
}

