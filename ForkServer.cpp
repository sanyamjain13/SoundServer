#include <iostream>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include<arpa/inet.h>
#include<map>
#include<vector>
#include <fstream>

#define SA struct sockaddr
#define LISTENQ 128
#define MAX 1024
using namespace std;


int main()
{
	int listenFd, connFd;
	pid_t childPid;
	socklen_t cliLen;
	struct sockaddr_in servaddr, cliaddr;
	
	
	//------------------------------------------------------------

	listenFd = socket(AF_INET, SOCK_STREAM, 0);
	cout<<"Socket successfully created :) \n";
	
	//------------------------------------------------------------

	bzero(&servaddr, sizeof(servaddr));
	//binding the ip and port of server in the socket structure
	servaddr.sin_port = htons(8080);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	bind(listenFd, (SA*) &servaddr, sizeof(servaddr));

    char ser[16];
	cout<<"Binding at IP -> "<<inet_ntop(AF_INET, &servaddr.sin_addr,ser,sizeof(ser))<<"\n";
    

	//------------------------------------------------------------
	
	listen(listenFd, LISTENQ);
	cout<<"Server listening Now :D \n";
	//------------------------------------------------------------


	for(;;)
	{
		cliLen = sizeof(cliaddr);
		connFd =accept(listenFd,(SA*)&cliaddr,&cliLen); 
        cout<<"client connected successfully";
		if(connFd<0) 
		{
			cout<<"Connection Lost! \n";
		}

		int n;
		char buff[MAX];
		if((childPid = fork()) == 0)
		{	
			//ip address of client will be stored in this
			char ipClient[16];

			//conversion of ip from nettwork to presentation
			const char* ptr=inet_ntop(AF_INET,&cliaddr.sin_addr,ipClient,sizeof(ipClient));
			cout<<"Connected to CLIENT -> ("<<ptr<<ntohs(cliaddr.sin_port)<<")\n";
			
			close(listenFd);

			for(;;)
			{
				bzero(buff,MAX);
				n=read(connFd,buff,sizeof(buff));

				if(n<0) 
				{
					cout<<"ERROR IN READING , EXITING\n";
					break;
				}
				// cout<<"Message from ( "<<ptr<<ntohs(cliaddr.sin_port)<<") : "<<buff;
                cout<<"client : "<<buff;
				bzero(buff,MAX);
				cout<<"SERVER : ";
				
				//while((buff[n++]=getchar())!="\n");
				fgets(buff,MAX,stdin);
				write(connFd,buff,sizeof(buff));
				
				if(strncmp("exit",buff,4)==0 || strncmp("EXIT",buff,4)==0) 
				{
					cout<<"\nExiting the child... \n";
					break;
				}
			}

			close(connFd);
			exit(0);
		
        }

		close(connFd);
	}

    return 1;
	
}