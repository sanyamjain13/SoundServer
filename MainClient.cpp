#include <iostream>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <sstream>

#include<stdlib.h>
#include<stdio.h>
#include<string>
#include<string.h>
#include<unordered_map>
#include<algorithm>

#define SA struct sockaddr
#define MAX 1024

using namespace std;
int main()
{
	int sockFd,loginStats=-1;
	struct sockaddr_in servaddr;
	char buff[MAX];

	//----------------------------------------------------------------------------
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	cout<<"__________________________________________________\n\n";
	if(sockFd == -1)
	{
		cout<<"\nSocket creation failed...\n";
		exit(0);
	}
	else
		cout<<"\nSocket successfully created :)\n\n";

	//----------------------------------------------------------------------------

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_port =htons(8080);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr =INADDR_ANY;
	//----------------------------------------------------------------------------
	
	if(connect(sockFd, (SA*) &servaddr, sizeof(servaddr)) != 0)
	{
		cout<<"Connection with the server failed...\n";
		exit(0);
	}
	else
		cout<<"Connected to the server Successfully :D \n";
	//----------------------------------------------------------------------------
	
	int n;
	for( ; ; )
	{
		//----------------------------------------------------------------------------
		
		//LOGIN MODULE OF CLIENT
		if(loginStats < 0)
		{	
			cout<<"\n__________________________________________________\n";
			cout<<"\nLogin With Your Credentials : \n\n";

			string id,password,login="";
			cout<<"UserId : ";
			getline(cin,id);
			
			cout<<"Password : ";
			getline(cin,password);

			login+=id+" "+password;
			
			std::copy(login.begin(),login.end(),buff);
			
			buff[login.size()]='\0';		
			
			n=write(sockFd,buff,sizeof(buff));

			int status=0;
			int result=read(sockFd,&status,sizeof(status));
			if(ntohl(status)>0)
			{
				cout<<"\nLogin Successfully ! \n";
				cout<<"\n-> Type 'sound' to Know if Server is Active or not ! \n";
				cout<<"__________________________________________________\n\n";
				loginStats=1;
			}

			else
			{
				cout<<"Sorry your ID & Password is not correct , EXITING ... ! \n";
				break;
			}

			//LOGIN MODULE OF CLIENT ENDS ...
		}
		//----------------------------------------------------------------------------
		
		//**************************************************************
		
		//CLIENT REQUEST HANDLING STARTS

		bzero(buff,MAX);
		cout<<"Client Message : ";
		
		//Getting input from the user....
		cin.getline(buff,sizeof(buff));

		//Storing the user input before the buffer gets cleared
		string userIp = buff;
		
		//Converting User Input to lowercase for avoiding further errors
		std::transform(userIp.begin(),userIp.end(),userIp.begin(),::tolower);

		//If the user input is : STORE , then take input from user as animal name & sound
		if(userIp.compare("store")==0)
		{	
			string animalName,animalSound,storeResult="";
			cout<<"Animal Name : ";
			getline(cin,animalName);

			cout<<"Animal Sound : ";
			getline(cin,animalSound);

			storeResult=userIp+" "+animalName+" "+animalSound;
			std::copy(storeResult.begin(),storeResult.end(),buff);
		}		

		//writing the userinput to the server for a response
		n=write(sockFd,buff,sizeof(buff));

		if(n<0) 			
		{
			cout<<"Error in Writing .. Try Again \n";
			continue;
		}

		//****************************************************************************
		
		//GETTING RESPONSE FROM THE SERVER IN THE BUFFER...

		bzero(buff,MAX);
		
		//Reading response of server from the socket
		n=read(sockFd,buff,sizeof(buff));

		//when Server sends no data, so prematurely exit;
		if(n<=0)
		{	
			cout<<"\n\nClosing Your Connection ... \n\n";
			break;
		}

		//Response from the server in this result;
		string result = buff;

		//###############################################################

		//if user wants to check if server is active, so it sends SOUND
		if(userIp.compare("sound")==0)
		{	
			cout<<"\nSERVER : "<<result<<"\n\n";
		}
		
		//###############################################################

		else if(userIp.compare("store")==0)
		{	
			cout<<"\n-------------------------------------------\n";

			if(result=="success") cout<<"Sound Successfully Stored ! :) \n";
			
			cout<<"-------------------------------------------\n";

		}

		//###############################################################

		else if (userIp.compare("query")==0) 
		{
			cout<<"\n================================== \n";

			if(result=="none") cout<<"I DONT KNOW ANY ANIMAL SOUND :( \nQUERY : OK\n";
			else cout<<"\nList of Animal Sounds I know : \n\n"<<result<<"\n>> QUERY : OK";
			
			cout<<"\n================================== \n";
			
		}

		//###############################################################

		else if (userIp.compare("bye")==0)
		{
			cout<<"Closing Your Connection with the server! Bye .. \n\n";
			break;
		}

		//###############################################################

		else if (userIp.compare("end")==0)
		{
			cout<<"\nENDING THE SERVER ..\n";
			break;
		}

		//###############################################################

		else
		{   
			cout<<"\n~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n\n";

			if(result=="none") 
				cout<<"Sorry! I dont Know '"<<userIp<<"'\n";
			
			else 
				cout<<"SOUND : A '"<<userIp<<"' SAYS '"<<result<<"'\n";

			cout<<"\n~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n\n";
		}

		//###############################################################

		//Request Handling Ends;
        //**************************************************************************************

	
	}

	close(sockFd);
	return 1;
}
