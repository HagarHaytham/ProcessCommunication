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

int disk_id;
int process_counter=0;
vector <msgbuff> message_log;
vector <msgbuff> disk_log;
vector <char *> response_log;
vector <int> disk_status_log;
vector <long> process_list;
int up_queue = msgget(disk_up, IPC_CREAT|0644);//99 for up // dummy values 
int down_queue = msgget(disk_down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys

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
	for(int i=0;i<2;i++)
	{
	int recieve = msgrcv(up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down
	if(first_message.mtext[0]=='D')
		disk_id=first_message.mtype;
	else if(first_message.mtext[0]=='P')
		process_list[process_counter++]=first_message.mtype;
	}
	cout<<"GOT The ID From DISK , READY FOR comm "<<disk_id<<endl;
}
int disk_status()
{
	struct msgbuff message;
	int recieve = msgrcv(up_queue, &message, sizeof(message.mtext),disk_id, !IPC_NOWAIT); 
	if(recieve != -1 )
	{
		return message.mtext;
		disk_status_log.push_back((int)message.mtext);
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
			kernel_response.mtype=message.mtype;
			message.mtype=disk_id;
			if(message.mtext[0] == 'D' && disk_response >10)
			{
				
				kernel_response.mtext[0]='1';
				int send = msgsnd(down_queue, &message, sizeof(message.mtext), IPC_NOWAIT);
				cout<<"message.mtext "<<message.mtext<<endl;
				disk_log.push_back(message);
				latency=1;
			}
			
			else	kernel_response.mtext[0]='3';
			if(message.mtext[0] == 'A' && disk_response <10)
			{
				
				
				kernel_response.mtext[0]='0';
				int send = msgsnd(down_queue, &message, sizeof(message.mtext), IPC_NOWAIT);
				cout<<"message.mtext "<<message.mtext<<endl;
				disk_log.push_back(message);
				latency=3;
			}
			else 	
			
				kernel_response.mtext[0]='2';
		
			int send = msgsnd(down_queue, &kernel_response, sizeof(message.mtext), IPC_NOWAIT);
			cout<<"kernel_response.mtext "<<kernel_response.mtext<<endl;
			response_log.push_back(kernel_response.mtext);
		}


	return 	latency;

}



int main()
{

	printf("Begin Of Kernel MAIN \n");
	initialize(up_queue,up_queue);
	
	int clk=-1;
	int current_time=0;
	int latency=0;
	struct msgbuff message;
	int prev_time=get_time();
	while(1)
{
	current_time=get_time();
	int recieve = msgrcv(up_queue, &message, sizeof(message.mtext),0, IPC_NOWAIT);  
	if(recieve !=-1 )
	{
	cout<<" recieved message "<<message.mtext<<endl;
	int i=0;
	for(i=0;i<process_list.size();i++)
		if(message.mtype == process_list[i].mtype)
			break;
	if(i==process_list.size())
	process_list[i].push_back();
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

