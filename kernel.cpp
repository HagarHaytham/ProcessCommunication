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
#include<fstream>
#include <cstring>
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
ofstream outputFile("log.txt");

int disk_id;
int process_counter=0;
int clk=-1;
vector <msgbuff> message_log;
vector <long> process_list;
int up_queue = msgget(disk_up, IPC_CREAT|0644);//99 for up // dummy values 
int down_queue = msgget(disk_down, IPC_CREAT|0644);// 100 for down // kernel should have the same keys
struct msgbuff message;

int get_time()
{
	seconds s = duration_cast< seconds >(system_clock::now().time_since_epoch());
	int time= s.count();
	return time;
}


void initialize()
{
	printf("didn't get The ID From DISK yet");
	struct msgbuff first_message;
	cout<<"up queue id = "<<up_queue<<endl;
	//for(int i=0;i<2;i++)
	//{
		int recieve = msgrcv(up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down
		if(first_message.mtext[0]=='D')
			disk_id=first_message.mtype;
		else if(first_message.mtext[0]=='P')
		{
			process_list.push_back(first_message.mtype);
			process_counter++;
		}
		////////
		recieve = msgrcv(up_queue, &first_message, sizeof(first_message.mtext),0, !IPC_NOWAIT);   // receive on up , send on down
		if(first_message.mtext[0]=='D')
			disk_id=first_message.mtype;
		else if(first_message.mtext[0]=='P')
		{
			process_list.push_back(first_message.mtype);
			process_counter++;
		}
	//}
	cout<<"GOT The ID From DISK , READY FOR comm "<<disk_id<<endl;
}
bool  canDelete(char * diskStatus,char slotNo)
{
	int idx =int(slotNo) -48;
	//char slot = diskStatus[idx];
	if (diskStatus[idx] == '0')// empty slot
		return false;
	return true;
}
bool  canAdd(char *diskStatus)
{
	int cnt =0;
	for (int i=0;i<10;i++)
	{
		if(diskStatus[i]=='0')// empty slot
			{
				cnt++;
				break;
			}
	}
	cout<<"Can Add Function  my cnt is : "<<cnt<<endl;
	if (cnt>0)
		return true;
	return false;
}
char * disk_status()
{
	// recieve msg : mtype hwa el disk status , msg hya el free slots 
	
	int recieve = msgrcv(up_queue, &message, sizeof(message.mtext),disk_id, !IPC_NOWAIT); 
	if(recieve != -1 )
	{
		outputFile << clk <<" disk Response "<<message.mtext<<endl;
		return message.mtext;
	}
	string str ="-1";
	return const_cast<char*>(str.c_str());
}
int process_request(struct msgbuff message1)
{
	int latency=0;

	struct msgbuff kernel_response;

	kill(disk_id,SIGUSR1);
	char * disk_response=disk_status();
	cout<<"disk_response "<<disk_response<<endl;

	if( disk_response!="-1" )
	{
		kernel_response.mtype=message1.mtype;
		message1.mtype=disk_id;
		if(message1.mtext[0] == 'D' && canDelete(disk_response,message1.mtext[1])) //modify
		{
			strcpy(kernel_response.mtext,"1");
			int send = msgsnd(down_queue, &message1, sizeof(message1.mtext), IPC_NOWAIT);
			cout<<"message1.mtext "<<message1.mtext<<endl;
			outputFile << clk <<" Msg to disk "<< message1.mtext << " Disk ID "<< message1.mtype<< endl;
			latency=1;
		}

		else if (message1.mtext[0] == 'D')
			strcpy(kernel_response.mtext,"3");

		else if(message1.mtext[0] == 'A' && canAdd(disk_response))
		{

			strcpy(kernel_response.mtext,"0");// successful  add 
			int send = msgsnd(down_queue, &message1, sizeof(message1.mtext), IPC_NOWAIT);

			cout<<"message1.mtext "<<message1.mtext<<endl;
			outputFile << clk <<" Msg to disk "<<message1.mtext<< " Disk ID "<< message1.mtype << endl;
			latency=3;
		}
		else 
			strcpy(kernel_response.mtext,"2");// unable to add 


		int send = msgsnd(down_queue, &kernel_response, sizeof(kernel_response.mtext), IPC_NOWAIT);
		cout<<"kernel_response.mtext "<<kernel_response.mtext<<endl;
		outputFile << clk <<" kernel_response "<< kernel_response.mtext <<" the id is :"<< kernel_response.mtype<< endl;
	}

	message_log.erase (message_log.begin());
	return 	latency;

}

int main()
{
	
	printf("Begin Of Kernel MAIN \n");
	outputFile<<"Kernel ID "<<getpid()<<endl;
	initialize();

	int current_time=0;
	int latency=0;
	//struct msgbuff message;
	int prev_time=get_time();
	while(1)
	{
		current_time=get_time();
		int recieve = msgrcv(up_queue, &message, sizeof(message.mtext),0, IPC_NOWAIT);  
		if(recieve !=-1 )
		{
			cout<<" request from process "<<message.mtext<<endl;
			int i=0;
			for(i=0;i<process_list.size();i++)
				if(message.mtype == process_list[i])
					break;
			if(i==process_list.size())
				process_list.push_back(message.mtype);
			if (message.mtext[0]!='P')
			{
				message_log.push_back(message);
				outputFile << clk <<" request from process " << message.mtext << "The ID is " << message.mtype<< endl;
			}
		}

		if(current_time-prev_time == 1)
		{

			prev_time = current_time;
			clk++;
			if(latency > 0)
				latency--;
			for(int i=0;i<process_list.size();i++)
				kill(process_list[i],SIGUSR2);
			kill(disk_id,SIGUSR2);
			for(int i=0;i<message_log.size();i++)
			{
				if(latency ==0)
					latency=process_request(message);
			}
		}	
	}

	outputFile.close();
	return 0;
}

