#include <iostream>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <utility>

#include<stdlib.h>
#include<stdio.h>
#include<string>
#include<string.h>
#include<unordered_map>
#include<algorithm>
#include<vector>

#define SA struct sockaddr
#define LISTENQ 128
#define MAX 1024
#define CLIENTSIZE 5
using namespace std;

//databse manager class
class animalSound
{
    public:        
        
        unordered_map<string,string>sound;
        
        animalSound()
        {
            sound["dog"]="woof";
            sound["cat"]="meow";
            sound["bear"]="growl";
            sound["horse"]="neigh";
            sound["chick"]="cluck";
        }

        int getDbSize()
        {
            return sound.size();
        }


        string getSound(string a)
        {   
            if(!sound.count(a)) return "none";
            return sound[a];
        }


        bool isEmpty()
        {
            if(sound.empty()) return true;
            return false;
        }


        bool addAnimalSound(string a,string s)
        {
            if( !sound.count(a) && sound.size()>15) return true;

            sound[a]=s;
            return true;
        }



        string getAnimals()
        {   
            string res="";
            if(sound.empty()) return res;
            int i=1,j=0;
            for(auto it=sound.begin() ; it!=sound.end(); it++)
            {   
                if((j+1)%3==0)
                {
                    res+=std::to_string(i)+". "+it->first+"\n\n";
                }
                else
                {
                    res+=std::to_string(i)+". "+it->first+"  ";
                }

                i++;
                j++;
            }

            return res;
        }
};

//---------------------------------------------------------------------------------

string lowerCase(string s)
{
    transform(s.begin(),s.end(),s.begin(),::tolower);
    return s;
}

string upperCase(string s)
{
    transform(s.begin(),s.end(),s.begin(),::toupper);
    return s;
}

//---------------------------------------------------------------------------------

int main()
{   
    /* 
    The variable "maxi" is the highest index in the client array that is currently in use and the 
    variable "maxfd" (plus one) is the current value of the first argument to select. 
     */
	//------------------------------------------------------------
    
    // Declaring All the variables used in the program
    int listenFd, connFd, sockfd, maxfd, maxi, nready, i;
    int Client[CLIENTSIZE],loginStatus[CLIENTSIZE];;
    
    unordered_map<string,string>users;
    unordered_map<int,int>userStatus;

    fd_set allset,rset;
	socklen_t cliLen;
	struct sockaddr_in servaddr, cliaddr;
	
     animalSound* a=new animalSound;
    
	//------------------------------------------------------------

	listenFd = socket(AF_INET, SOCK_STREAM, 0);
    cout<<"_________________________________\n";

	cout<<"\nSocket successfully created :) \n";
	
	//------------------------------------------------------------

	bzero(&servaddr, sizeof(servaddr));
	//binding the ip and port of server in the socket structure
	servaddr.sin_port = htons(8080);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	bind(listenFd, (SA*) &servaddr, sizeof(servaddr));

    char ser[16];
	cout<<"\nBinding at IP : "<<inet_ntop(AF_INET, &servaddr.sin_addr,ser,sizeof(ser))<<"\n";
    

	//------------------------------------------------------------
	
	listen(listenFd, LISTENQ);
	cout<<"\nServer listening Now :D \n";

    cout<<"_________________________________\n\n";

	//------------------------------------------------------------

    maxfd=listenFd;
    maxi=-1;
    for(i=0; i < CLIENTSIZE; i++)
    {
        Client[i]=-1;
        loginStatus[i]=-1;
    }

    FD_ZERO(&allset);
    FD_SET(listenFd,&allset);
	//------------------------------------------------------------

    //HANDLING MULTIPLE CLIENTS USING SELECT SYSTEM CALL
    
    /* int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,const struct timeval *timeout);
            Returns: positive count of ready descriptors, 0 on timeout, –1 on error 

            void FD_ZERO(fd_set *fdset);            clear all bits in fdset 
            void FD_SET(int fd, fd_set *fdset);     turn on the bit for fd in fdset 
            void FD_CLR(int fd, fd_set *fdset);     turn off the bit for fd in fdset 
            int FD_ISSET(int fd, fd_set *fdset);    is the bit for fd on in fdset ? 
    */

   for( ; ;)
   {    
       rset=allset;

       //nready will store the number of ready descriptors
       nready=select(maxfd+1, &rset, NULL, NULL, NULL);

	//----------------------------------------------------------------------------------
    
        //IF NEW CLIENT CONNECTION

        /* 
            When the first client establishes a connection with our server, the listening descriptor
            becomes readable and our server calls accept.
         */

       if(FD_ISSET(listenFd,&rset))
       {
            cliLen=sizeof(cliaddr);
            connFd =accept(listenFd,(SA*)&cliaddr,&cliLen);
            
            if(connFd < 0)
            {
                cout<<"Client Not Connected .. \n";
                continue;
            }
            
            //cout<<"Connection FD : "<<connFd<<endl;
            //ip address of client will be stored in this
            char ipClient[16];
            cout<<"___________________________________________\n";

            //conversion of ip from nettwork to presentation
            const char* ptr=inet_ntop(AF_INET,&cliaddr.sin_addr,ipClient,sizeof(ipClient));
            cout<<"\nConnected to CLIENT -> ( "<<ptr<<ntohs(cliaddr.sin_port)<<" )\n";

            cout<<"___________________________________________\n";
            /* 
                From this point on, our server must remember the new connected socket in its client
                array, and the connected socket must be added to the descriptor set.
            */

            for(i=0; i<CLIENTSIZE; i++)
            {    
                //Finding first empty slot for the new descriptor
                if(Client[i]<0)
                {
                    Client[i]=connFd;
                    userStatus[connFd]=-1;
                    break;
                }
            }

            if(i==CLIENTSIZE)
            {
                cout<<"Too Many Clients! "<<endl;
                close(connFd);
                continue;
            }

            //adding new decriptor to the set
            FD_SET(connFd,&allset);

            //updating the maximum number of descriptors in set
            if(connFd > maxfd) maxfd=connFd; 

            //cout<<"\nNumber Of Descriptors Active : "<<nready<<endl;

            //updating maximum index in the client array
            if(i > maxi) maxi=i;

            if(--nready <= 0)
            {   
                //coz 1st descriptor is the mastersocket for listening server
                continue;
            }
        }

        //----------------------------------------------------------------------------------

        //CHECKING FOR THE DATA FROM THE CLIENT

        for(i=0; i<=maxi; i++)
        {   

            if( (sockfd = Client[i]) < 0 ) continue;

            if(FD_ISSET(sockfd,&rset))
            {   
                char buff[MAX];
                bzero(buff,MAX);
                
                int n=read(sockfd,buff,sizeof(buff));

                if(n==0)
                {       
                    //connection closed by client;
                    cout<<"\n\n********************************************";
                    cout<<"\nClient "<<i+1<<" has not entered any input"<<endl;
                    cout<<"\nClosing Connection with Client "<<i+1<<" !! \n";
                    
                    /* When our server reads this connected socket, if read returns 0. We then close this socket 
                    and update our data structures accordingly. The value of client [i] is set to –1 and 
                    sockfd in the descriptor set is set to 0 */

                    close(sockfd);
                    FD_CLR(sockfd,&allset);
                    Client[i]=-1;
                    userStatus[sockfd]=-1;
                }

                else
                {   
                    //------------------------------------------------------------------------

                    //LOGIN MODULE
                    if(userStatus[sockfd]<0)
                    {
                        stringstream userDetails(buff);
                        string id,pass,loginInput;

                        //array that stores the user login details after trimming of white spaces
                        vector<string>details;
                        
                        //removing extra whitespaces from the login details (userId , Password)
                        while(getline(userDetails,loginInput,' '))
                        {
                            if(loginInput.size()==0) continue;
                            details.push_back(loginInput);
                        }
                        
                        //converting userid , pass in lowercase to avoid errors
                        id=lowerCase(details[0]);
                        pass=lowerCase(details[1]);

                        int status=-1; 

                        cout<<"\n--------------------------------\n";
                        cout<<"\nCredentials Entered by Client : "<<i+1<<endl;
                        cout<<"\nClient : "<<i+1<<" UserID => "<<id<<endl;
                        cout<<"Client : "<<i+1<<" Password => "<<pass<<endl;
                        cout<<"\n--------------------------------\n";

                        
                        if(!users.count(id))
                        {   
                            cout<<"\nRegistering client : "<<i+1<<endl;
                            users[id]=pass;
                            userStatus[sockfd]=1;
                            status=htonl(1); // 1: New user registered
                            write(sockfd,&status,sizeof(status));
                            cout<<"\n--------------------------------\n";
                        }

                        else if(users[id]==pass)
                        {   
                            cout<<"Client :"<<i+1<<" Logged into the server!"<<endl;
                            userStatus[sockfd]=1;
                            status=htonl(2); //2: user logged in
                            write(sockfd,&status,sizeof(status));
                            cout<<"--------------------------------\n";
                        }

                        else
                        {   
                            cout<<"Credentials Incorrect by => Client : "<<i+1<<endl;
                            userStatus[sockfd]=-1;
                            status=htonl(3); //3: user details wrong
                            write(sockfd,&status,sizeof(status));

                            cout<<"-----------------------------------\n";
                        }

                    }

                    //LOGIN MODULE ENDS
                    //--------------------------------------------------------------------------
                    
                    //Handling of CLIENT REQUESTS (QUERY , STORE, END , BYE, ANIMAL SOUND) 
                    else
                    {
                        cout<<"\nClient "<<i+1<<" : "<<buff<<"\n";
                        
                        //Request from the Client
                        string userIp=lowerCase(buff);
                        
                        stringstream ss(userIp);
                        vector<string>input;
                        
                        while(getline(ss,userIp,' '))
                        {
                            if(userIp.size()==0) continue;
                            input.push_back(userIp);
                        }

                        userIp=input[0];
                        
                        bzero(buff,MAX);
                        
                        //***********************************************************************
                        
                        //if client wants to check if server is active
                        
                        if(userIp.compare("sound")==0)
                        {
                            cout<<"\nUSER : Hello Server Are You there ? \n";
                            string ok="Hey! I am there :D \n";
                            cout<<ok<<endl;

                            std::copy(ok.begin(), ok.end(),buff);
                            write(sockfd,buff,sizeof(buff));
                        }

                        //***********************************************************************

                        //WHEN WE ARE STORING NEW ANIMAL SOUND IN THE DATABASE
                        else if(userIp.compare("store")==0)
                        {   
                            
                            if(input.size()!=3)
                            {   
                                write(1,"Failed to store!\n",sizeof("Failed to store!\n"));
                                write(sockfd,"fail",sizeof("fail"));
                            }

                            else
                            {
                                string animal,sound;

                                animal=lowerCase(input[1]);
                                sound=lowerCase(input[2]);

                                if(a->addAnimalSound(animal,sound))
                                {  
                                    string res="success";
                                    std::copy(res.begin(),res.end(),buff);
                                    write(sockfd,buff,sizeof(buff));
                                }
                            }
                        }

                        //***********************************************************************

                        //WHEN CLIENT WANTS TO KNOW WHICH ANIMAL SOUNDS DO WE KNOW

                        else if (userIp.compare("query")==0) 
                        {
                            string result= a->getAnimals();
                            if(result=="") result="none";
                            cout<<"\n"<<result<<"\n\n";
                            std::copy(result.begin(), result.end(),buff);
                            write(sockfd,buff,sizeof(buff));
                        }

                        //***********************************************************************

                        //WHEN A CLIENT WANTS TO TERMINATE CONNECTION WITH THE SERVER
                        else if (userIp.compare("bye")==0)
                        {   
                            shutdown(sockfd,SHUT_RDWR);
                            close(sockfd);
                            FD_CLR(sockfd,&allset);
                            Client[i]=-1;
                            userStatus.erase(sockfd);
                            cout<<"\nClient "<<i+1<<" says BYE! CLosing Connection..\n";
                            cout<<"_________________________________________________\n";
                        }

                        //***********************************************************************

                        //WHEN A CLIENT ENDS THE SERVER
                        else if (userIp.compare("end")==0)
                        {   
                            //CLOSING ALL THE ACTIVE CLIENTS , UNSETTING THEIR SOCKFDS
                            for(int j=0 ; j<=maxi; j++)
                            {
                                if(Client[j]<0) continue;
                               
                                sockfd=Client[j];
                              
                                cout<<"Closing client :"<<j+1<<endl;
                                shutdown(sockfd,SHUT_RDWR);
                                close(sockfd);
                                FD_CLR(sockfd,&allset);
                                Client[j]=-1;
                                userStatus.erase(sockfd);
                                cout<<"------------------------------\n\n";
                            
                            }
                            
                            //NOW SHUTTING DOWN THE SERVER FINALLY
                            shutdown(listenFd,SHUT_RDWR);
                            FD_ZERO(&allset);
                            close(listenFd);                          
                            
                            cout<<"\nSERVER CLOSED BY CLIENT : "<<i+1<<"\n";
                            cout<<"___________________________________\n";
                            exit(0);
                        }

                        //***********************************************************************

                        //HANDLING THR ANIMAL SOUNDS
                        else
                        {   
                            cout<<"\nSOUND : "<<a->getSound(userIp)<<endl;
                            string result= a->getSound(userIp);
                            std::copy(result.begin(),result.end(),buff);
                            write(sockfd,buff,sizeof(buff));
                        }

                        //***********************************************************************

                            
                    }
                    //------------------------------------------------------------------------
                }

                if(--nready<=0)
                {
                    //cout<<"\nNo more Readable Descriptors \n ";
                    break;
                }
            }
        }
        //----------------------------------------------------------------------------------
   }

   close(listenFd);
   return 1;
}