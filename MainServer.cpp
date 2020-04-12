//SOUNDSERVER

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

#define SA struct sockaddr
#define LISTENQ 128
#define MAX 1024
#define CLIENTSIZE 5
using namespace std;

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
            int i=1;
            for(auto it=sound.begin() ; it!=sound.end(); it++)
            {
                res+=std::to_string(i)+". "+it->first+"\n";
                i++;
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

//Inserting CLients UserId & Password 
void initialise_Users(string* u,string* p )
{
    for(int i=0; i<CLIENTSIZE; i++)
    {
        int j=i+1;
        string user="user"+std::to_string(i+1);
        string password="pass"+std::to_string(i+1);
        u[i]=user;
        p[i]=password;
    }

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
    int Client[FD_SETSIZE],loginStatus[CLIENTSIZE];;
    string *user=new string[CLIENTSIZE];
    string *password=new string[CLIENTSIZE];

    if(!user || !password) 
    {
        cout<<"Allocation of memory failed !"<<endl;
        return 0;
    }

    fd_set allset,rset;
	socklen_t cliLen;
	struct sockaddr_in servaddr, cliaddr;
	
     animalSound* a=new animalSound;
    //------------------------------------------------------------
	
    initialise_Users(user,password);
    
	//------------------------------------------------------------

	listenFd = socket(AF_INET, SOCK_STREAM, 0);
    cout<<"___________________________________________\n";

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

    cout<<"___________________________________________\n\n";

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

            //ip address of client will be stored in this
            char ipClient[16];
            
            cout<<"___________________________________________\n";

            //conversion of ip from nettwork to presentation
            const char* ptr=inet_ntop(AF_INET,&cliaddr.sin_addr,ipClient,sizeof(ipClient));
            cout<<"\nConnected to CLIENT -> ( "<<ptr<<ntohs(cliaddr.sin_port)<<" )\n";

            cout<<"___________________________________________\n\n";


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
                    break;
                }
            }

            if(i==CLIENTSIZE)
            {
                cout<<"Too Many Clients! "<<endl;
                //continue;
                return 0;
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
            //cout<<"Reading : \n";
            if( (sockfd = Client[i]) < 0 ) continue;

            if(FD_ISSET(sockfd,&rset))
            {   
                char buff[MAX];
                bzero(buff,MAX);
                //cout<<a->sound["dog"]<<endl;
                int n=read(sockfd,buff,sizeof(buff));
                if(n==0)
                {       
                    //connection closed by client;
                    cout<<"Closing Connection with Client "<<i+1<<" !!! \n\n";
                    cout<<"________________________________________________________________\n";
                    
                    /* When our server reads this connected socket, if read returns 0. We then close this socket 
                    and update our data structures accordingly. The value of client [i] is set to –1 and 
                    sockfd in the descriptor set is set to 0 */

                    close(sockfd);
                    FD_CLR(sockfd,&allset);
                    Client[i]=-1;
                    loginStatus[i]=-1;
                }

                else
                {   
                    //------------------------------------------------------------------------

                    //LOGIN MODULE
                    if(loginStatus[i]<0)
                    {
                        stringstream userDetails(buff);
                        string id,pass;
                        getline(userDetails,id,' ');
                        getline(userDetails,pass,' ');
                        
                        cout<<"\nCLIENT "<<i+1<<" UserID :- "<<id<<endl;
                        cout<<"CLIENT "<<i+1<<" Password :- "<<pass<<"\n\n";
                        if(user[i]==lowerCase(id) && password[i]==lowerCase(pass))
                        {
                            //bzero(buff,MAX);
                            int num=2;
                            int success=htonl(num);
                            write(sockfd,&success,sizeof(success));
                            loginStatus[i]=1;
                        }

                        else
                        {
                            int stat= shutdown(sockfd,SHUT_RDWR);
                            FD_CLR(sockfd,&allset);
                            Client[i]=-1;
                            loginStatus[i]=-1;
                            if(stat==0) cout<<"Successfully Closed the connection !  \n";
                        }

                    }

                    //LOGIN MODULE ENDS
                    //--------------------------------------------------------------------------
                    
                    //Handling of CLIENT REQUESTS (QUERY , STORE, END , BYE, ANIMAL SOUND) 
                    else
                    {
                        cout<<"Client "<<i+1<<" : "<<buff<"\n";
                        
                        //Request from the Client
                        string userIp=lowerCase(buff);
                        stringstream ss(userIp);
                        getline(ss,userIp,' ');

                        bzero(buff,MAX);
                        
                        //***********************************************************************
                        
                        //if client wants to check if server is active
                        if(userIp.compare("sound")==0)
                        {
                            cout<<"\nUSER : Hello Server Are You there ? \n";
                            string ok="SERVER : Yes SoundServer is Active :-))\n\n";
                            cout<<ok<<endl;

                            std::copy(ok.begin(), ok.end(),buff);
                            write(sockfd,buff,sizeof(buff));
                        }

                        //***********************************************************************

                        //WHEN WE ARE STORING NEW ANIMAL SOUND IN THE DATABASE
                        else if(userIp.compare("store")==0)
                        {   
                            //Tokenising 
                            string animal,sound;
                            getline(ss,animal,' ');
                            getline(ss,sound,' ');
                            
                            std::transform(animal.begin(),animal.end(),animal.begin(),::tolower);
                            std::transform(sound.begin(),sound.end(),sound.begin(),::tolower);
                            // cout<<"Animal : "<<animal<<endl;
                            // cout<<"Sound : "<<sound<<endl;
                            if(a->addAnimalSound(animal,sound))
                            {  
                                string res="success";
                                std::copy(res.begin(),res.end(),buff);
                                write(sockfd,buff,sizeof(buff));
                            }
                        
                        }

                        //***********************************************************************

                        else if (userIp.compare("query")==0) 
                        {
                            string result= a->getAnimals();
                            if(result=="") result="none";
                            cout<<"QUERY RESULT : \n"<<result<<"\n\n";
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
                            loginStatus[i]=-1;
                            cout<<"\nClient "<<i+1<<" says BYE! CLosing Connection..\n\n";
                        }

                        //***********************************************************************

                        //WHEN A CLIENT ENDS THE SERVER
                        else if (userIp.compare("end")==0)
                        {   
                            //CLOSING ALL THE ACTIVE CLIENTS , UNSETTING THEIR SOCKFDS
                            for(int j=0 ; j<=maxi; j++)
                            {
                                if(Client[j]<0) continue;
                                
                                int tempFd=Client[j];
                                char temp[MAX]; 
                                if(FD_ISSET(tempFd,&rset))
                                {
                                    shutdown(tempFd,SHUT_RDWR);
                                    close(tempFd);
                                    FD_CLR(tempFd,&allset);
                                    Client[j]=-1;
                                    loginStatus[j]=-1;
                                }
                            }
                            
                            //NOW SHUTTING DOWN THE SERVER FINALLY
                            shutdown(listenFd,SHUT_RDWR);
                            FD_ZERO(&allset);
                            close(listenFd);

                            delete user;
                            delete password;                            
                            
                            cout<<"\n\n SERVER CLOSED BY CLIENT "<<i+1<<"\n\n";
                            exit(0);
                        }

                        //***********************************************************************

                        //HANDLING THR ANIMAL SOUNDS
                        else
                        {   
                            cout<<"SOUND : "<<a->getSound(userIp)<<endl;
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
