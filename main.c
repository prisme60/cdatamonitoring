/*
 * main.c
 * 
 * Copyright 2017 christian <christian@coolermaster>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h> 
#include <sys/un.h> 
#include <unistd.h>

#include "circularBuffer.h"
#include "historic.h"

#define SEPARATOR_JSON_FIELD ",\n"

typedef enum {
	QUEUE_MINUTE,
	QUEUE_HOUR,
	QUEUE_DAYS,
	QUEUE_NBELEMENTS
} tEQueues;

int initSocket();
void treatSocket(int sockfd, tHistoricItem historics[], int nbHistorics);
void writeAllHistoricToSocket(int newsockfd, tHistoricItem historics[], int nbHistorics);

int main(int argc, char *argv[]) {
    tData dataMinute[32];
    tData dataHour[128];
    tData dataDays[9192];
    tHistoricItem historicQueues[QUEUE_NBELEMENTS] = {{.limit = 24},{.limit = 120},{.limit = 9192}};
    int sockfd = initSocket(argc >=2 ? argv[1] : NULL);
    queue_init(&historicQueues[QUEUE_MINUTE].circularBuffer, dataMinute, sizeof(dataMinute)/sizeof(dataMinute[0]));
    queue_init(&historicQueues[QUEUE_HOUR  ].circularBuffer, dataHour  , sizeof(dataHour)  /sizeof(dataHour[0]));
    queue_init(&historicQueues[QUEUE_DAYS  ].circularBuffer, dataDays  , sizeof(dataDays)  /sizeof(dataDays[0]));
    printf("Enter loop\n");
    while(true) {
		tData data;
        queue_putItem(&historicQueues[QUEUE_MINUTE].circularBuffer, retrieveDynamicData(&data));
		//printf("nbElements (MINUTE) = %d\tnbElements (HOUR) = %d\tnbElements (DAYS) = %d\n",
		//	queue_getNbItems(&historicQueues[QUEUE_MINUTE].circularBuffer),
		//	queue_getNbItems(&historicQueues[QUEUE_HOUR  ].circularBuffer),
		//	queue_getNbItems(&historicQueues[QUEUE_DAYS  ].circularBuffer));
        reduceHistoric(historicQueues, QUEUE_NBELEMENTS);
        treatSocket(sockfd, historicQueues, QUEUE_NBELEMENTS);
    }

    return 0;
}

int initSocket(char socketPath[]) {
	int sockfd;

	/* Initialize socket structure */
	struct sockaddr_un	 serv_addr = {
		.sun_family = AF_UNIX,
		.sun_path = "socket"
	};
	if(socketPath != NULL) {
		strncpy(serv_addr.sun_path, socketPath, sizeof(serv_addr.sun_path)-1);
	}
	unlink(serv_addr.sun_path);
   
	/* First call to socket() function */
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}

	/* Now start listening for the clients, here process will
	* go in sleep mode and will wait for the incoming connection
	*/
	listen(sockfd,5);
	
	return sockfd;
}

void treatSocket(int sockfd, tHistoricItem historics[], int nbHistorics) {
	fd_set rfds;
	struct timeval tv = {
		.tv_sec = 5,
		.tv_usec = 0
	};
	int retval;
	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);
	retval = select(sockfd+1, &rfds, NULL, NULL, &tv);
	/* Don’t rely on the value of tv now! */

	if (retval == -1)
		perror("select()");
	else if (retval)
	{
		//printf("Data is available now.\n");
		if(FD_ISSET(sockfd, &rfds))
		{
			//int n;
			//char buffer[256] = {0};
			/* Accept actual connection from the client */
			int newsockfd = accept(sockfd, NULL, 0);

			if (newsockfd < 0) {
				perror("ERROR on accept");
				exit(1);
			}

			/* If connection is established then start communicating */
			//n = read(newsockfd, buffer, sizeof(buffer)-1);

			//if (n < 0) {
			//	perror("ERROR reading from socket");
			//	exit(1);
			//}

			//printf("Here is the message: %s\n",buffer);

			/* Write all json to the client */
			writeAllHistoricToSocket(newsockfd, historics, nbHistorics);
			/* No more information to send, so we can close the client socket*/
			close(newsockfd);
		}
	}
	//else
	//	printf("No data within five seconds.\n");
}

void writeAllHistoricToSocket(int newsockfd, tHistoricItem historics[], int nbHistorics) {
	bool firstSeparator = true;
	// Write only '['
	write(newsockfd,&"[",1);
	for(int i = nbHistorics-1; i >= 0; i--) {
		int nbElements = queue_getNbItems(&historics[i].circularBuffer);
		for(int index = 0; index < nbElements; index++) {
			tData itemValue;
			char str[240];
			char *pStr = str;
			if(queue_peekItem(&historics[i].circularBuffer, index, &itemValue)) {
				int remainingSize = sizeof(str);
				if(tData_json(&itemValue, &pStr, &remainingSize) != NULL) {
					if(!firstSeparator) {
						write(newsockfd, SEPARATOR_JSON_FIELD, sizeof(SEPARATOR_JSON_FIELD)-1);
					}
					// Minus one is for '\0' (we mustn't send it!)
					int nbChars = sizeof(str) - remainingSize - 1;
					printf("Write size = %d\n", nbChars);
					int n = write(newsockfd, str, nbChars);

					if (n < 0) {
						perror("ERROR writing to socket");
						return;
					}
					firstSeparator = false;
				}
			}
		}
	}
	// Write only ']'! Don't write "\0" !!!!
	write(newsockfd,&"]",1);
}

