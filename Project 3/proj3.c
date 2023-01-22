

/*
Nicolas Slavonia
njs140
proj3.c
October 19th, 2022
Project 3, web server
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
#include <unistd.h>


#define ERROR 1
#define QLEN 1
#define PROTOCOL "tcp"
#define BUFLEN 8192
#define miniBUF 256
#define tinyBUF 16

//variables
bool foundP, foundR, foundT;
char *port, *directory, *token, *response;


/* The exit method, given from the class Socket code */
void errexit (char *format, char *arg){
	fprintf (stderr,format,arg);
	fprintf (stderr,"\n");
	exit (ERROR);
}

// listens for a response and writes back
void listenResponse(int sd){
    struct sockaddr addr;
    unsigned int addrlen;
    int sd2, size, index = 0;
    char *buffer = malloc(BUFLEN), *response = malloc(BUFLEN);
    char *validURL = "HTTP/";
    char *parts[miniBUF], method[miniBUF],argument[miniBUF], location[miniBUF], http[miniBUF];
    char *ptr;
    FILE *sp, *fp;

    // listens for a connection
    if (listen (sd, QLEN) < 0)  errexit ("ERROR: cannot listen on port %s\n", port);

    /* accept a connection */
    addrlen = sizeof (addr);
    sd2 = accept(sd,&addr,&addrlen);
    if (sd2 < 0) errexit ("error accepting connection", NULL);

    // open the connection
    sp = fdopen(sd2,"rb");
    if (sp < 0) errexit ("ERROR: failed to open response",NULL);
    if (fgets(buffer,BUFLEN,sp) < 0)
            errexit ("ERROR: error reading the file: %s", NULL);

    // parse the first line
    ptr = strtok(buffer," ");
    while (ptr != NULL){
        parts[index] = ptr;
        ptr = strtok(NULL," ");
        index++;
    }
    // check if the first line is valid
    if (index != 3 || parts[2][strlen(parts[2])-2] != '\r' || parts[2][strlen(parts[2])-1] != '\n'){
        response = "HTTP/1.1 400 Malformed Request\r\n\r\n";
        if (write(sd2,response,strlen(response)) < 0)
            errexit ("ERROR: error writing message: %s", response);
        close(sd2);
        listenResponse(sd);
    } 
    strcpy(method,parts[0]);
    strcpy(argument,parts[1]);
    strcpy(http,parts[2]);
    buffer[0] = '\0';
    // parse every remaining line
    while (strcmp(buffer,"\r\n")){
        if (fgets(buffer,BUFLEN,sp) < 0)
            errexit ("ERROR: error reading the file: %s", NULL);
        // check if it is a valid line
        if (buffer[strlen(buffer)-2] != '\r' || buffer[strlen(buffer)-1] != '\n'){
            response = "HTTP/1.1 400 Malformed Request\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            close(sd2);
            listenResponse(sd);
        }
    }
    // check if the protocol is correct
    for (int i = 0; i < strlen(validURL); i++){
        if (http[i] != validURL[i]) {
            response = "HTTP/1.1 501 Protocol Not Implemented\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            close(sd2);
            listenResponse(sd);
        }
    }
    // if it is terminate
    if (!strcmp(method,"TERMINATE")) {
        if (!strcmp(argument,token)){
            response = "HTTP/1.1 200 Server Shutting Down\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            close(sd);
            close(sd2);
            exit(0);
        } else {
            response = "HTTP/1.1 403 Operation Forbidden\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            close(sd2);
            listenResponse(sd);
        }
        // if it is get
    } else if (!strcmp(method,"GET")){
        if (argument[0] != '/'){
            response = "HTTP/1.1 406 Invalid Filename\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            close(sd2);
            listenResponse(sd);
        }
        if (!strcmp(argument,"/")) strcpy(argument,"/homepage.html");
        strcpy(location,directory);
        // check if file exists
        if (access(strcat(location,argument),F_OK) == 0){
            fp = fopen(location,"rb");
            if (fp < 0) errexit ("ERROR: cannot open file %s\n", NULL);
            response = "HTTP/1.1 200 OK\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            // write the contents to the connection
		    size = fread(buffer, 1, sizeof(buffer), fp); 
		    while (size != 0){
                if (write(sd2,buffer,size) < 0)
                    errexit ("ERROR: error writing message: %s", NULL);
			    memset(buffer,0x0,BUFLEN);
			    size = fread(buffer, 1, sizeof(buffer), fp); 
		    }
		    memset(buffer,0x0,size);
		    fclose(fp);
            close(sd2);
            listenResponse(sd);
        } else {
            response = "HTTP/1.1 404 File Not Found\r\n\r\n";
            if (write(sd2,response,strlen(response)) < 0)
                errexit ("ERROR: error writing message: %s", response);
            close(sd2);
            listenResponse(sd);
        }
    } else {
        response = "HTTP/1.1 405 Unsupported Method\r\n\r\n";
        if (write(sd2,response,strlen(response)) < 0)
            errexit ("ERROR: error writing message: %s", response);
        close(sd2);
        listenResponse(sd);
    }
    close(sd2);
}

// makes the socket, binds to a port and then calls the listenResponse method and listens
void makeSocket(){
    struct sockaddr_in sin;
    struct protoent *protoinfo;
    int sd;

    /* determine protocol */
    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL)
        errexit ("ERROR: cannot find protocol information for %s", PROTOCOL);

    /* setup endpoint info */
    memset ((char *)&sin,0x0,sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons ((u_short) atoi (port));

    /* allocate a socket */
    sd = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (sd < 0) errexit("ERROR: cannot create socket", NULL);

    /* bind the socket */
    if (bind (sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) 
		errexit ("ERROR: cannot bind to port %s", port);

    /* listen for incoming connections */
    listenResponse(sd);

    /* close connections and exit */
    close(sd);
    exit(0);
}

/* main method, collects the given information and decided what to do with it */
int main(int argc, char* argv[]){
	int unknown = 0;
	// if we were given nothing
	if (argc == 1) errexit("ERROR: not enough information was given", NULL);
	// check through every element and compare what it is
	for(int i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-p")){
			foundP = true;
			port = argv[i+1];
		} else if (!strcmp(argv[i], "-r")){
			foundR = true;
			directory = argv[i+1];
		} else if (!strcmp(argv[i], "-t")){
            foundT = true;
            token = argv[i+1];
        } 
		else unknown++;
	}
	if (unknown > 4) errexit("ERROR: gave too many commands", NULL);
	// make sure we got -p, -t, -r
	if (foundP == false) errexit("ERROR: the -p option is mandatory", NULL);
	if (foundR == false) errexit("ERROR: the -r option is mandatory", NULL);
    if (foundT == false) errexit("ERROR: the -t option is mandatory", NULL);
	// if we did, parse the url and make the socket
	if (atoi(port) < 1025 || atoi(port) > 65535) errexit("ERROR: invalid port number", NULL);
    makeSocket();
}
