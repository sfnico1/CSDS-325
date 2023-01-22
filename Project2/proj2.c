

/*
Nicolas Slavonia
njs140
proj2.c
October 3rd, 2022
Project 2, web client
*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdbool.h>

#define ERROR 1
#define PORT "80"
#define PROTOCOL "tcp"
#define BUFLEN 8192
#define miniBUF 256
#define tinyBUF 16

//variables
bool foundU, foundO, foundI, foundC, foundS, foundCC  = false;
char *fileName;
char *hostURL;
char *fileURL;

/* Prints the -i command information using printf statements */
void minusI(){
	printf("INF: hostname = %s\n",hostURL);
	printf("INF: web_filename = %s\n", fileURL);
	printf("INF: output_filename = %s\n",fileName);
}


/* Prints the -c command information using printf statements */
void minusC(){
	if (foundCC == false) printf("REQ: GET %s HTTP/1.0\r\n", fileURL);
	else printf("REQ: GET %s HTTP/1.1\r\n", fileURL);
	printf("REQ: Host: %s\r\n", hostURL);
	printf("REQ: User-Agent: CWRU CSDS 325 Client 1.0\r\n");
}


/* The exit method, given from the class Socket code */
void errexit (char *format, char *arg){
	fprintf (stderr,format,arg);
	fprintf (stderr,"\n");
	exit (ERROR);
}

/* breaks up the given URL into the host and file path, also checks to see if it is a valid url (http) */
void parseURL(char *argv){
	char *temp = malloc(strlen(argv) + tinyBUF);
	char *parts[tinyBUF];
	char *ptr = strtok(argv,"/");
	char *validURL = "http:";
	int index = 0;

	// checking to see if it is an http: request
	while (ptr[index] != '\0'){
		if (tolower(ptr[index]) != validURL[index]) errexit("ERROR: not a valid http request",NULL);
		index++;
	} 
	index = 0;
	// breaking each path given into separate parts
	while (ptr != NULL){
		ptr = strtok(NULL,"/");
		parts[index] = ptr;
		index++;
	}
	index = 1;
	temp[0] = '\0';
	strcat(temp,"/");
	// putting the paths back together to make the fileURL
	while (parts[index] != NULL){
		strcat(temp,parts[index]);
		if (parts[index + 1] != NULL) strcat(temp,"/");
		index++;
	}
	if (parts[1] != NULL && parts[2] == NULL) strcat(temp,"/");
	hostURL = parts[0];
	fileURL = temp;
}


/* Socket code, based off of the given Socket code, connects to the server and then checks the response */
void makeSocket(){
	struct sockaddr_in sin;
	struct hostent *hinfo;
	struct protoent *protoinfo;
	char buffer[BUFLEN];
	char *request = malloc(strlen(hostURL) + strlen(fileURL) + miniBUF); 
	char *location = malloc(miniBUF);
	char *str = malloc(BUFLEN + 1);
	int sd, wr, size;

	hinfo = gethostbyname(hostURL);
	if (hinfo == NULL) errexit ("ERROR: cannot find name: %s", hostURL);

	memset ((char *)&sin, 0x0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons (atoi (PORT));
	memcpy ((char *)&sin.sin_addr,hinfo->h_addr,hinfo->h_length);

	if ((protoinfo = getprotobyname (PROTOCOL)) == NULL) 
		errexit ("ERROR: cannot find protocol information for %s", PROTOCOL);

	sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
	if (sd < 0) errexit("ERROR: cannot create socket",NULL);

	if (connect (sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) errexit ("ERROR: cannot connect", NULL);

	// creating the request
	strcat(request, "GET ");
	strcat(request, fileURL); 
	if (foundCC == false) strcat(request, " HTTP/1.0\r\n");
	else strcat(request, " HTTP/1.1\r\n");
	strcat(request, "Host: ");
	strcat(request, hostURL);
	strcat(request, "\r\nUser-Agent: CWRU CSDS 325 Client 1.0\r\n\r\n");

	wr = write(sd, request, strlen(request));
	if (wr < 0) errexit ("ERROR: writing error",NULL);

	FILE *sp = fdopen(sd,"rb");
	fgets(str, BUFLEN, sp);
	// getting and printing the header reposne
	if (foundS == true) printf("RSP: %s",str);

	char *status = strtok(str," ");
	status = strtok(NULL," ");
	
	// use status to see if it was a 200 reponse 
	if (!strcmp(status, "200")){
		fgets(str, BUFLEN, sp);
		// print the header, otherwise gloss over it
		while (strcmp(str, "\r\n")){
			if (foundS == true) printf("RSP: %s",str);
			fgets(str, BUFLEN, sp);
		}	
		FILE *fp = fopen(fileName,"wb");
		size = fread(buffer, 1, sizeof(buffer), sp); 
		// collect the data into the opened file
		while (size != 0){
			fwrite(buffer, 1, size, fp);
			memset(buffer,0x0,BUFLEN);
			size = fread(buffer, 1, sizeof(buffer), sp); 
		}
		memset(buffer,0x0,size);
		fclose(fp);
	// 301 response, moved permanently
	} else if (!strcmp(status, "301")){
		fgets(str, BUFLEN, sp);
		// print the header while also finding the new location
		while (strcmp(str, "\r\n")){
			if (foundS == true) printf("RSP: %s",str);
			str = strtok(str," ");
			if (!strcmp(str, "Location:")) strcpy(location, strtok(NULL," "));
			fgets(str, BUFLEN, sp);
		}
		// parse the new location and reprint -i and -c
		parseURL(location);
		fileURL = strtok(fileURL,"\r");
		if (foundI == true) minusI();
		if (foundC == true) minusC();
		// close the socket and make a new one
		close(sd);
		makeSocket();
	} else {
		fgets(str, BUFLEN, sp);
		// bad response, print the header and quit
		while (strcmp(str, "\r\n")){
			if (foundS == true) printf("RSP: %s",str);
			fgets(str, BUFLEN, sp);
		}
		errexit ("ERROR: response error, non 200/301 response",NULL);
	}
	close (sd);
}


/* main method, collects the given information and decided what to do with it */
int main(int argc, char* argv[]){
	int posURL;
	int unknown = 0;
	// if we were given nothing
	if (argc == 1) errexit("ERROR: not enough information was given", NULL);
	// check through every element and compare what it is
	for(int i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-u")){
			foundU = true;
			posURL = i + 1;
		} else if (!strcmp(argv[i], "-o")){
			foundO = true;
			fileName = argv[i+1];
		} else if (!strcmp(argv[i], "-i")) foundI = true;
		else if (!strcmp(argv[i], "-c")) foundC = true;
		else if (!strcmp(argv[i], "-s")) foundS = true;
		else if (!strcmp(argv[i], "-C")) foundCC = true;
		else unknown++;
	}
	if (unknown > 2) errexit("ERROR: gave too many commands", NULL);
	// make sure we got -u and -o
	if (foundU == false) errexit("ERROR: the -u option is mandatory", NULL);
	if (foundO == false) errexit("ERROR: the -o option is mandatory", NULL);
	// if we did, parse the url and make the socket
	if (foundU == true && foundO == true){
		parseURL(argv[posURL]);
		if (foundI == true) minusI();
		if (foundC == true) minusC();
		makeSocket();
	}
}


