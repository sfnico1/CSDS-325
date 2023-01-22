
/*
Nicolas Slavonia
njs140
proj4.c
November 10th, 2022
Project 4, packet traces
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define ERROR 1
#define rowTobyte 4
#define udpHL 8
#define MAX_PKT_SIZE 1600
#define SIZE 50069



//variables
int ipCounter = 0;
double lastTime;
char *traceFile;
bool foundT, foundS, foundL, foundP, foundM = false;
bool nonIP, validIP, validP, ackBit = false;
struct meta_info meta;
struct DataItem* htable[SIZE]; 



/* The exit method, given from the class Socket code */
void errexit (char *format, char *arg){
	fprintf (stderr,format,arg);
	fprintf (stderr,"\n");
	exit (ERROR);
}


/* meta information, using same layout as trace file */
struct meta_info{
    unsigned short caplen;
    unsigned short ignored;
    unsigned int secs;
    unsigned int usecs;
};


/* record of information about the current packet */
struct pkt_info{
    unsigned short caplen;      /* from meta info */
    double now;                 /* from meta info */
    unsigned char pkt [MAX_PKT_SIZE];
    struct ether_header *ethh;  /* ptr to ethernet header, if fully present,
                                   otherwise NULL */
    struct iphdr *iph;          /* ptr to IP header, if fully present, 
                                   otherwise NULL */
    struct tcphdr *tcph;        /* ptr to TCP header, if fully present,
                                   otherwise NULL */
    struct udphdr *udph;        /* ptr to UDP header, if fully present,
                                   otherwise NULL */
};


// this is the item in the hashtable (it is a linkedlist)
struct DataItem{
    int sIP; 
    int dIP;
    int traffic; 
    struct DataItem* inner;  
};


// inserting a new item into the hashtable
void insert(int sourceIP ,int destIP, int traffic) {
    struct DataItem *entry = (struct DataItem*) malloc(sizeof(struct DataItem));
    entry->sIP = sourceIP;
    entry->dIP = destIP;
    entry->traffic = traffic;
    entry->inner = NULL;
    int hashIndex = (sourceIP + destIP) % SIZE;
    if (hashIndex < 0) hashIndex = hashIndex*(-1);
    
    if (htable[hashIndex] == NULL) htable[hashIndex] = entry;
    else { // if it is not null, iterate through the linked list to find it
        struct DataItem *dummy = htable[hashIndex];
        while (dummy != NULL && (dummy->sIP != sourceIP || dummy->dIP != destIP)) dummy = dummy->inner;
        if (dummy != NULL) dummy->traffic = dummy->traffic + traffic;
        else {
            entry->inner = htable[hashIndex];
            htable[hashIndex] = entry;
        }
    }
}


/* fd - an open file to read packets from
   pinfo - allocated memory to put packet info into for 1 packet

   returns:
   1 - a packet was read and pinfo is setup for processing the packet
   0 - we have hit the end of the file and no packet is available 
 */
unsigned short next_packet (int fd, struct pkt_info *pinfo){
    int bytes_read;
    double usecTosec = 1000000.0;

    memset (pinfo,0x0,sizeof (struct pkt_info));
    memset (&meta,0x0,sizeof (struct meta_info));

    /* read the meta information */
    bytes_read = read(fd,&meta,sizeof (meta));
    if (bytes_read == 0)
        return 0;
    if (bytes_read < sizeof (meta))
        errexit ("ERROR: cannot read meta information",NULL);
    
    pinfo->now = (ntohl (meta.secs)) + (ntohl (meta.usecs))/usecTosec;
    lastTime = pinfo->now;
    pinfo->caplen = ntohs (meta.caplen);

    /* set pinfo->now based on meta.secs & meta.usecs */
    if (pinfo->caplen == 0)
        return 1;
    if (pinfo->caplen > MAX_PKT_SIZE)
        errexit ("ERROR: packet too big",NULL);
    
    /* read the packet contents */
    bytes_read = read (fd,pinfo->pkt,pinfo->caplen);
    if (bytes_read < 0)
        errexit ("ERROR: error reading packet",NULL);
    if (bytes_read < pinfo->caplen)
        errexit ("ERROR: unexpected end of file encountered",NULL);
    if (bytes_read < sizeof (struct ether_header))
        return 1;

    pinfo->ethh = (struct ether_header *)pinfo->pkt;
    pinfo->ethh->ether_type = ntohs (pinfo->ethh->ether_type);
    
    if (pinfo->ethh->ether_type != ETHERTYPE_IP) {
        nonIP = true;
        return 1;
    } else ipCounter++;
    if (pinfo->caplen == sizeof (struct ether_header)){
        validIP = true;
        return 1;
    }

    /* set pinfo->iph to start of IP header */
    pinfo->iph = (struct iphdr *)(pinfo->pkt + sizeof(struct ether_header));
    pinfo->iph->tot_len = ntohs (pinfo->iph->tot_len);

    if (pinfo->caplen > (sizeof(struct ether_header) + sizeof(struct iphdr))){
        validP = true;
        if (pinfo->iph->protocol == SOL_TCP){
            pinfo->tcph = (struct tcphdr *)(pinfo->pkt + (sizeof(struct ether_header) + pinfo->iph->ihl*rowTobyte));
            pinfo->tcph->source = ntohs (pinfo->tcph->source);
            pinfo->tcph->dest = ntohs (pinfo->tcph->dest);
            pinfo->tcph->window = ntohs (pinfo->tcph->window);
            pinfo->tcph->seq = ntohl (pinfo->tcph->seq);
            if (pinfo->tcph->ack == 1){
                ackBit = true;
                pinfo->tcph->ack_seq = ntohl (pinfo->tcph->ack_seq);
            }
        } else if (pinfo->iph->protocol == SOL_UDP){
            pinfo->udph = (struct udphdr *)(pinfo->pkt + (sizeof(struct ether_header) + pinfo->iph->ihl*rowTobyte));
            pinfo->udph->len = ntohs (pinfo->udph->len);
        }
    }
    return 1;
}


// prints summary info about the entire file
void summaryMode(){
    double total = 1;
    int fp = open(traceFile,O_RDONLY);
    if (fp < 0) errexit("ERROR: unable to open tracefile",NULL);
    struct pkt_info nexth;
    next_packet(fp, &nexth);
    printf("FIRST PKT: %f\n", nexth.now);
    while (next_packet(fp, &nexth)) total++;
    printf("LAST PKT: %f\nTOTAL PACKETS: %.0f\nIP PACKETS: %d\n", lastTime, total, ipCounter);
}


// print information regarding IP packets
void lengthMode(){
    int fp = open(traceFile,O_RDONLY);
    if (fp < 0) errexit("ERROR: unable to open tracefile",NULL);
    struct pkt_info nexth;
    while (next_packet(fp, &nexth)){
        if (nonIP == false){ // it is IP
            printf("%.6f %.0hu ", nexth.now, nexth.caplen);
            if (validIP == false){ // it has IP
                printf("%.0hu %d ", nexth.iph->tot_len, nexth.iph->ihl*rowTobyte);
                if (nexth.iph->protocol == SOL_TCP){ // it is TCP
                    printf("T ");
                    if (validP == true) // it has TCP
                        printf("%d %d\n", nexth.tcph->doff*rowTobyte, nexth.iph->tot_len-nexth.iph->ihl*rowTobyte-nexth.tcph->doff*rowTobyte);
                    else printf("- -\n");
                }
                else if (nexth.iph->protocol == SOL_UDP){ // it is UDP
                    printf("U ");
                    if (validP == true) // it has UDP
                        printf("%d %d\n", udpHL, nexth.udph->len - udpHL);
                    else printf("- -\n");
                }
                else 
                    printf("? ? ?\n");
                validP = false; // reset
            } else printf("- - - - -\n");
            validIP = false; // reset 
        }
        nonIP = false; // reset
    }
}


// print info regarding TCP packets
void printingMode(){
    int fp = open(traceFile,O_RDONLY);
    if (fp < 0) errexit("ERROR: unable to open tracefile",NULL);
    struct in_addr ip_addr;
    struct pkt_info nexth;
    while (next_packet(fp, &nexth)){
        if (validP == true && nexth.iph->protocol == SOL_TCP){
            ip_addr.s_addr = nexth.iph->saddr;
            printf("%.6f %s ", nexth.now, inet_ntoa(ip_addr));
            ip_addr.s_addr = nexth.iph->daddr;
            printf("%s %d %d %d %d %u", inet_ntoa(ip_addr), nexth.iph->ttl,nexth.tcph->source,nexth.tcph->dest,nexth.tcph->window,nexth.tcph->seq);
            if (ackBit == true) printf(" %u\n", nexth.tcph->ack_seq);
            else printf(" -\n");
            ackBit = false;
        }
        validP = false;
    }
}


// traffic matrix mode - uses a hashtable of linkled lists to keep track of source/dest IP pairs
// first make the hashtable, then print it
void matrixMode(){
    memset(htable,0x0,SIZE*(sizeof(struct DataHash *)));
    int fp = open(traceFile,O_RDONLY);
    if (fp < 0) errexit("ERROR: unable to open tracefile",NULL);
    struct in_addr ip_addr;
    struct pkt_info nexth;
    struct DataItem *dummy;
    while (next_packet(fp, &nexth)){
        if (validP == true && nexth.iph->protocol == SOL_TCP){ // insert if TCP
            insert(nexth.iph->saddr,nexth.iph->daddr,nexth.iph->tot_len-nexth.iph->ihl*rowTobyte-nexth.tcph->doff*rowTobyte);
        }
        validP = false;
    }
    for(int i = 0; i < SIZE; i++){
        if(htable[i] != NULL){
            dummy = htable[i];
	        while (dummy != NULL){
                ip_addr.s_addr = dummy->sIP;
                printf("%s ", inet_ntoa(ip_addr));
                ip_addr.s_addr = dummy->dIP;
                printf("%s %d\n", inet_ntoa(ip_addr), dummy->traffic);
                dummy = dummy->inner;
            }
        }
    }
}


/* main method, collects the given information and decided what to do with it */
int main(int argc, char* argv[]){
    int unknown, counter = 0;
	// if we were given nothing
	if (argc == 1) errexit("ERROR: not enough information was given", NULL);
	// check through every element and compare what it is
	for(int i = 1; i < argc; i++){
		if (!strcmp(argv[i], "-t")){
			foundT = true;
			traceFile = argv[i+1];
            counter++;
		} else if (!strcmp(argv[i], "-s")){
            foundS = true;
            counter++;
        } else if (!strcmp(argv[i], "-l")){
            foundL = true;
            counter++;
        } else if (!strcmp(argv[i], "-p")){ 
            foundP = true;
            counter++;
        } else if (!strcmp(argv[i], "-m")){ 
            foundM = true;
            counter++;
        } else unknown++;
	}
    // error checking
	//if (unknown > 1) errexit("ERROR: gave too many unknown commands", NULL);
    if (foundT == false) errexit("ERROR: the -t option is mandatory", NULL);
    if (counter > 2) errexit("ERROR: gave too many mode commands", NULL);
    if (counter <= 1) errexit("ERROR: gave too few mode commands", NULL);

    if (foundS == true) summaryMode();
    else if (foundL == true) lengthMode();
    else if (foundP == true) printingMode();
    else if (foundM == true) matrixMode();
}



