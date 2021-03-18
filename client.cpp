#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <vector>
#include <string>
#include <sys/wait.h>
#include <iostream>
#include <pthread.h>
using namespace std;

/* A client-server program that uses posix sockets to communicate */

struct message
{
    char network[20];
    char broadcast[20];
    char minhost[20];
    char maxhost[20];
    int numhosts;
};

struct memory
{
    string lines = "";
    string network = "";
    string broadcast = "";
    string hostmin = "";
    string hostmax = "";
    string numhosts = "";
    char* servername;
    int portno = 0;
    memory()
    {
        lines = "";
        network = "";
        broadcast = "";
        hostmin = "";
        hostmax = "";
        numhosts = "";
        servername;
        portno = 0;
    }
    /*I am using this constructor to read from the cin, create enough memory allocation, (6 vector
    elements if 6 lines present), but leave the necessary parameters (network, broadcast, etc.) empty for 
    updating later */
    memory(string myline)
    {
        lines = myline;
        network = "";
        broadcast = "";
        hostmin = "";
        hostmax = "";
        string numhosts = "";
        servername;
        portno = 0;
    }
    memory(string myline, char* server, int port)
    {
        lines = myline;
        network = "";
        broadcast = "";
        hostmin = "";
        hostmax = "";
        string numhosts = "";
        servername = server;
        portno = port;
    }
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
void* analyzer(void* x_void_ptr)
{
    /* IN THE CHILD THREAD, I AM GETTING THE SERVER NAME & PORT NO,
    AND CONNECTING IT WITH THE SERVER.CPP. */
    char buffer[256]; //THIS IS TO SEND THE IP & SUBNET TO THE SERVER FUNCTION
    memory* x_ptr = (memory*) x_void_ptr;
    char* server_temp = x_ptr->servername;
    int portno, sockfd, n;
    portno = x_ptr->portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(server_temp);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
    bzero(buffer,256);

    strcpy(buffer, x_ptr->lines.c_str());
    n = write(sockfd, &buffer, 255); 
    if (n < 0) error("ERROR writing to socket");
    
    /*OVER HERE I AM TAKING THE VALUES FROM MY SERVER FUNCTION. 
    I AM USING A DIFFERENT STRUCT, BECAUSE PASSING STRINGS ARE VERY HARD TO IMPLEMENT
    (WHEN THE SIZEOF(STRUCT) CHANGES, MY READ() FUNCTION DOES NOT UPDATE ITSELF), 
    CHAR ARRAYS ARE EASIER TO WORK WITH*/
    message to_read;
    n = read(sockfd, &to_read, sizeof(to_read)); 
    if (n < 0) error("ERROR reading from socket");

    x_ptr->network = string(to_read.network);
    x_ptr->broadcast = string(to_read.broadcast);
    x_ptr->hostmin = string(to_read.minhost);
    x_ptr->hostmax = string(to_read.maxhost);
    x_ptr->numhosts = to_string(to_read.numhosts);

    return  NULL;
}

int main(int argc, char *argv[])
{
    string line = "";
    vector<memory> storage;
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    int portname;
    char* server_id;
    portname = atoi(argv[2]);
    server_id = argv[1];
     
    while (getline(cin, line))
    {
        
        memory temp;
        temp.lines = line;
        temp.servername = server_id;
        temp.portno = portname;
        storage.push_back(temp);
        /*Over here, I am reading line by line and creating necessary allocation in vector storage*/
    }
   
    int threadcount = storage.size();
    pthread_t tid[100];
    for (int i = 0; i < threadcount; i++) {
        pthread_create(&tid[i], NULL, analyzer, &storage[i]);
    }
    for (int i = 0; i < threadcount; i++) {
        pthread_join(tid[i], NULL);
    }

    for (int i = 0; i < threadcount; i++) {
        string temp = storage.at(i).lines;
        string tempip = (temp.substr(0, temp.find(' ')));
        temp.erase(0, temp.find(' ')+1);
        cout << "IP Address: " << tempip << endl;
        cout << "Subnet: " << temp << endl;
        cout << "Network: " << storage.at(i).network << endl;
        cout << "Broadcast: " << storage.at(i).broadcast << endl;
        cout << "HostMin: " << storage.at(i).hostmin << endl;
        cout << "HostMax: " << storage.at(i).hostmax << endl;
        if (i != threadcount - 1){
            cout << "# Hosts: " << storage.at(i).numhosts << endl << endl;
        }
        else cout << "# Hosts: " << storage.at(i).numhosts;
    }

    return 0;
}
