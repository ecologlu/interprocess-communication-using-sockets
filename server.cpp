#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <sys/wait.h>
#include <cmath>
#include <iostream>
#include <pthread.h>
using namespace std;


struct message
{
    char network[20];
    char broadcast[20];
    char minhost[20];
    char maxhost[20];
    int numhosts;
};


int countbits(uint8_t n)
{
    //this function is to find the 0's in a 8bit value, while loop finds 1's, then I substract from 8
	//inspired from: https://www.geeksforgeeks.org/count-set-bits-in-an-integer/
	unsigned int count = 0;
	while (n) {
		count += n & 1;
		n >>= 1;
	}
	return 8-count;
}

void fireman(int)
{
   while (waitpid(-1, NULL, WNOHANG) > 0) //-1 wait for any process
      std::cout << "A child process ended" << std::endl;
}


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, n;
    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2) { //CHECKING ARGUMENT COUNT
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding"); 
    listen(sockfd, 25); //MAX 25 CONNECTIONS CAN BE QUEUED
    clilen = sizeof(cli_addr);
    signal(SIGCHLD, fireman);
    while (1)
    {
        char buffer[256];
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        if (fork() == 0) 
        {
            bzero(buffer, 255);
            n = read(newsockfd, buffer, 40); //GETTING THE LINES, I AM USING A STATIC VARIABLE FOR SIZE, AS OUR LINE WON'T BE MORE THAN ~30 CHARACTERS (IP + SUBNET)
            if (n<0)
                error("ERROR reading from socket");
            string temp = string(buffer);
            string ip = (temp.substr(0, temp.find(' ')));
            temp.erase(0, temp.find(' ')+1);
            string subnet = temp;
            string tempnetwork, tempbroadcast, temphostmin, temphostmax = "";
            for (int i = 0 ; i < 4; i++) {
                string octet1 = (ip.substr(0,ip.find('.')));
                string octet2 = (subnet.substr(0,subnet.find('.')));
                if (i != 3) {
                    ip.erase(0, ip.find('.')+1);
		            subnet.erase(0, subnet.find('.')+1);
                }
                uint8_t tempoctet1 = stoi(octet1);
                uint8_t tempoctet2 = stoi(octet2);
                uint8_t tempnetworkint = (tempoctet1 & tempoctet2);
                uint8_t temptranslator = ~(tempoctet2);
                uint8_t tempbroadcastint = (tempnetworkint | temptranslator);

                if ( i != 3) {
                    tempnetwork += to_string(tempnetworkint) + '.';
                    tempbroadcast += to_string(tempbroadcastint) + '.';
                    temphostmin += to_string(tempnetworkint) + '.';
                    temphostmax += to_string(tempbroadcastint) + '.';
                }
                else {
                    tempnetwork += to_string(tempnetworkint);
                    tempbroadcast += to_string(tempbroadcastint);
                    temphostmin += to_string(tempnetworkint + 1);
                    temphostmax += to_string(tempbroadcastint - 1);
                }
            }
            //HERE I AM CALCULATING # OF HOSTS, NOTHING INTERESTING
            string new_subnet = temp;
            int count = 0;
            for (int m = 0; m < 3; m++) {
		        uint8_t temphosts = stoi((new_subnet.substr(0,new_subnet.find('.'))));
		        new_subnet.erase(0, new_subnet.find('.')+1);
		        count += countbits(temphosts);
	        }
            uint8_t temphosts = stoi(new_subnet);
            count += countbits(temphosts);
            int temp_numhosts = int(pow(2.0, count)-2);
            //CALCULATED NUMBER OF HOSTS
            
            struct message sending;
            strcpy(sending.network, tempnetwork.c_str());
            strcpy(sending.broadcast, tempbroadcast.c_str());
            strcpy(sending.minhost, temphostmin.c_str());
            strcpy(sending.maxhost, temphostmax.c_str());
            sending.numhosts = temp_numhosts;

            n = write(newsockfd, &sending, sizeof(message)); 
            if (n < 0) error("ERROR writing to socket");
            close(newsockfd);
        }
        wait(0);
        close(newsockfd);
    }
    close(sockfd);
    return 0;
}