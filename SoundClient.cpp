#include <iostream>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include<vector>

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
    int sockFd,loginStats=-1,stdineof,maxfd;
    fd_set rset;
    struct sockaddr_in servaddr;
    char buff[MAX];

    //----------------------------------------------------------------------------
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    cout<<"\n______________________________________\n";
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
        cout<<"Connected to the server Successfully! \n";
    
    //----------------------------------------------------------------------------
    
    FD_ZERO(&rset);
    stdineof=0;
    int n;

	cout<<"\n______________________________________\n";
	
	cout<<"\nLogin With Your Credentials : \n\n";
	
	write(1,"UserId : ",sizeof("UserId : "));

    for( ; ; )
    {
		
		FD_SET(fileno(stdin),&rset); //setting stdin file descriptor

        FD_SET(sockFd,&rset); //setting the socket descriptor
        
        maxfd = std::max(fileno(stdin),sockFd) + 1;
		
		select(maxfd , &rset , NULL, NULL , NULL);
	
        //----------------------------------------------------------------------------
        
		//When input from user is readable
        if(FD_ISSET(fileno(stdin),&rset))
        {   

			//if user is not yet logged in into the system

			if(loginStats < 0)
			{   
				string id,password,login="";
				
				getline(cin,id);
				
				//Here write(1) means it is showing output to the screen (stdout);
				write(1,"Password : ",sizeof("Password : "));

				getline(cin,password);


				//if(user has not entered the userid or password)
				if(id.size() == 0 || password.size() == 0)
				{
					write(1,"\nUserId : ",sizeof("\nUserId : "));
					continue;
				}


				login+=id+" "+password;
				
				std::copy(login.begin(),login.end(),buff);

				buff[login.size()]='\0';        

				//Sending the login details to the server
				write(sockFd,buff,sizeof(buff));
		
			}
        	
			//if already logged in , login status is not < 0
			else
			{	

				bzero(buff,MAX);

				//user input or user request
				string userIp;

				//Getting input from the user....
				cin.getline(buff,sizeof(buff));
				
				//Storing the user input before the buffer gets cleared
				userIp=buff;
				
				//if empty request
				if(userIp.size() ==0 ) 
				{	
					write(1,"\nClient Message : ",sizeof("\nClient Message : "));
					continue;
				}

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
				write(sockFd,buff,sizeof(buff));
				
			}

		}
        
		//---------------------------------------------------------------------------------

		//if output from the server is readable

        if(FD_ISSET(sockFd,&rset))
        {	
			//if user has entered the details, now checking the status   
            if(loginStats < 0)
            {
                int status=0;
                n = read(sockFd, &status, sizeof(status));
                
				//if server gets too many clients, so it closes the client sockfd
				if(n==0)
				{	
					write(1,"\n\n__Server Overloaded !!__\n\n",sizeof("\n\n__Server Overloaded !!__\n\n"));
					close(sockFd);
					exit(0);
				}

				/*status=1 : NEW USER 
				  status=2 : OLD USER   
				  status=3 : INCORRECT DETAILS 
				*/
                
				//1
                if(ntohl(status)==1)
                {
                    cout<<"\n~~ Welcome to the SoundServer ~~!\n\nYou are successfully Registered  !:) \n";
                    cout<<"\nNow you are an active user of 'SoundServer' !! \n";
                    cout<<"\n>> Type 'sound' to Know what operations I can do! \n";
                    cout<<"____________________________________________________\n\n";
                    loginStats=1;
                }
				
				//2
                else if(ntohl(status)==2)
                {
                    cout<<"\n~~ Welcome User ~~\n\nSuccessfully Logged in!  \n";
                    cout<<"\n-> Type 'sound' to Know what operations I can do! ! \n";
                    cout<<"____________________________________________________\n\n";

                    loginStats=1;
                }
				
				//3
                else
                {
                    char alert[]= "\nUserid or Password incorrect, Please Try Again.. \n\n";
					write(1,alert,strlen(alert));

                    bzero(buff,MAX);
                    loginStats=-1;

					char title[]="\nUserId : ";
					write(1,title,strlen(title));
    
                }
            }
			
			//if user is successfully logged in , now user request output comes here
			else
			{           
				//user input or request is stored in this variable 
				string userIp = buff;

				vector<string>input;

				//tokenising user ip string , and removing any spaces if in between

				stringstream ss(userIp);
				while(getline(ss,userIp,' '))
				{   
					if(userIp.size()==0) continue;
					input.push_back(userIp);
				}
				
				//in vector 1st index is the request [sound,query,store,end,bye]
				userIp=input[0];

				//clearing the buffer for storing the output of the server
				bzero(buff,MAX);

				//reading the output from server
				n=read(sockFd , buff , sizeof(buff));
				

				//if server did'nt write anything on the socket
				if(n==0)
				{	
					if(userIp.compare("bye")==0)
					{
						cout<<"\n~~ Closing Connection With the Server, Bye! ~~ \n\n";
						close(sockFd);
						exit(0);
					}

					else
					{
						cout<<"~~ Server Terminated Prematurely, See ya! ~~\n\n";
						close(sockFd);
						exit(0);						
					}
					                
				}

				//Response from the server in this result;
		
				string result = buff;
				
				//###############################################################

				//if user wants to check if server is active, so it sends SOUND

				if(userIp.compare("sound")==0)
				{   
					cout<<"_________________________________________________________\n";
					cout<<"\nSoundServer : "<<result<<"\n";

					cout<<">> Type 'query' to get the list of Animals I know.\n";
					cout<<">> Type ~ Animal name ~ to know the Sound. \n";
					cout<<">> Type 'store' to store a new Animal Sound.\n";
					cout<<">> Type 'bye' to end connection with me.\n";
					cout<<">> Type 'end' to close all the connections and the server\n";
					cout<<"\n_________________________________________________________\n";

				}
				
				//###############################################################
				
				//if user wants to add new sound in database 

				else if(userIp.compare("store")==0)
				{   
					cout<<"\n----------------------------------\n";

					if(result=="success") cout<<"Sound Successfully Stored ! :) \n";

					else if(result=="fail") cout<<"Missing Arguments , Try Again \n";
					
					cout<<"----------------------------------\n\n";

				}

				//###############################################################

				//if user wants to know which animals are there in database

				else if (userIp.compare("query")==0) 
				{
					cout<<"\n================================== \n";

					if(result=="none") cout<<"I DONT KNOW ANY ANIMAL SOUND :( \nQUERY : OK\n";

					else cout<<"\nList of Animal Sounds I know : \n\n"<<result<<"\n\n>> QUERY : OK";
					
					cout<<"\n================================== \n";
					
				}

				//###############################################################

				else
				{   
					//Sounds of animals comes here or other input outputs;

					cout<<"\n~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n";

					if(result=="none") 
						cout<<"Sorry! I dont Know '"<<userIp<<"'\n";
					
					else 
						cout<<"SOUND : A '"<<userIp<<"' SAYS '"<<result<<"'\n";

					cout<<"~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~\n\n";
				}

				//###############################################################

				//Request Handling Ends;
				
			}
			
			//stdout tp screen, for the input for client now
			write(1,"\nClient Message : ",sizeof("\nClient Message : "));
			
        }      
    }

    close(sockFd);
    return 1;
}