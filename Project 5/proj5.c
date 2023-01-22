
/*
Nicolas Slavonia
njs140
proj4.c
November 19th, 2022
Project 5, network analysis 
*/

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <time.h>

#define ERROR 1
#define BUFLEN 64


/* The exit method, given from the class Socket code */
void errexit (char *format, char *arg){
	fprintf (stderr,format,arg);
	fprintf (stderr,"\n");
	exit (ERROR);
}


// wget code
int main(int argc, char* argv[]){
    int counter = atoi(argv[1]);
    int size;
    char *request = malloc(5000); 
    char *response = malloc(5000); 
    char *ip[10];
    ip[0] = "https://case.edu"; // case.edu - here - 100K 
    ip[1] = "https://www.stanford.edu"; // stanford.edu - cali - 120K 
    ip[2] = "https://www.gov.za"; // gov.za - south africa - 60K 
    ip[3] = "https://www.kyoto-u.ac.jp"; // japan.jp - tokyo - 180K 
    ip[4] = "https://www.unimelb.edu.au"; // unimelb.edu.au - oceana australia - 40K 
    ip[5] = "https://www.nyse.com/index"; // nyse - 132K * 
    ip[6] = "https://g1.globo.com/?utm_source=globo.com&utm_medium=menuburguer"; // globo.com - brazil - 132K
    ip[7] = "koora.com"; // koora.com - France - 68K
    ip[8] = "https://www.alliance4creativity.com/about-us/"; // futbollibre.net - Finland - 40.0K
    ip[9] = "https://www.infobae.com/ultimas-noticias-america/"; // infobae.com - India Mumbai - 224K
    FILE *fp = fopen("test.out","wb");
    if (fp < 0) errexit("ERROR: cannot open file",NULL);
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);
    int startTime = tv.tv_sec;
    int tempTime;
    struct tm *today;
    today = localtime(&tv.tv_sec);
    char *time = malloc(100); 
    while (counter > 0){
        gettimeofday(&tv,&tz);
        tempTime = tv.tv_sec;
        sprintf(time, "%ld", tv.tv_sec - startTime);
        strcat(response, time);
        for (int i = 0; i < 10; i++){
            gettimeofday(&tv,&tz);
            strcat(request, "wget -q -O holder.txt ");
            strcat(request, ip[i]);
            system(request);
            strcat(response, " ");
            sprintf(time, "%ld.%ld", tv.tv_sec - tempTime, tv.tv_usec);
            strcat(response, time);
            request[0] = '\0';
        }
        fprintf(fp, "%s\n", response);
        response[0] = '\0';
        counter--;
        sleep(300); 
    }
    fclose(fp);
}



// curl code
int main(int argc, char* argv[]){
    sleep(600);
    int counter = atoi(argv[1]);
    char buffer[BUFLEN];
    int size;
    char *request = malloc(5000); 
    char *response = malloc(5000); 
    char *ip[13];
    ip[0] = "https://case.edu"; // case.edu - here 
    ip[1] = "https://www.stanford.edu"; // stanford.edu - cali 
    ip[2] = "https://www.gov.za"; // gov.za - south africa
    ip[3] = "https://www.kyoto-u.ac.jp"; // japan.jp - tokyo
    ip[4] = "https://www.unimelb.edu.au"; // unimelb.edu.au - oceana australia 
    ip[5] = "https://www.nyse.com/index"; // nyse
    ip[6] = "https://www.globo.com"; // globo.com - brazil
    ip[7] = "koora.com"; // koora.com - France
    ip[8] = "https://www.alliance4creativity.com"; // futbollibre.net - Finland
    ip[9] = "https://www.infobae.com"; // infobae.com - India Mumbai
    ip[10] = "https://canvas.case.edu";
    ip[11] = "https://sis.case.edu";
    ip[12] = "https://mail.google.com";
    FILE *fp = fopen("test.out","wb");
    if (fp < 0) errexit("ERROR: cannot open file",NULL);
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);
    int startTime = tv.tv_sec;
    int tempTime;
    struct tm *today;
    today = localtime(&tv.tv_sec);
    char *time = malloc(100); 
    while (counter > 0){
        gettimeofday(&tv,&tz);
        tempTime = tv.tv_sec;
        sprintf(time, "%ld", tv.tv_sec - startTime);
        strcat(response, time);
        for (int i = 0; i < 13; i++){
            strcat(request, "curl -w \"%{time_namelookup} %{time_connect} %{time_starttransfer} %{time_total}\" > holder.txt -o /dev/null -s ");
            strcat(request, ip[i]);
            system(request);
            size = fread(buffer, 1, sizeof(buffer), fopen("holder.txt","rw")); 
            strcat(response, " ");
            buffer[size-2] = '\0';
            strcat(response, buffer);
            fprintf(fp, "%s\n", response);
            memset(buffer,0x0,size);
            request[0] = '\0';
            response[0] = '\0';
        }
        counter--;
        sleep(600);
    }
    fclose(fp);
}


// binary search proxy server code
int main(int argc, char* argv[]){
    char *request = malloc(1000);
    strcat(request, "curl -w \"%{time_namelookup}\" > holder.txt -o /dev/null -s https://www.kyoto-u.ac.jp");
    double max = 600;
    double min = 300;
    double time = 450;
    double dns;
    char buffer[BUFLEN];
    int size;
    int converged = 0;
    while (converged == 0){
        system(request);
        size = fread(buffer, 1, sizeof(buffer), fopen("holder.txt","rw")); 
        dns = strtod(buffer, NULL);
        if (dns > 0.1){
            max = time;
            time = min + (max - min)/2;
        } 
        else {
            min = time;
            time = min + (max - min)/2;
        } 
        printf("DNS = %f, time = %f\n", dns, time);
        sleep(time);
    }
}


// ping code
int main(int argc, char* argv[]){ 
    int counter = atoi(argv[1]);
    char buffer[BUFLEN];
    int size;
    char *request = malloc(1000); 
    char *response = malloc(1000); 
    char *ip[11];
    ip[0] = "129.22.12.21"; // case.edu - here 
    ip[1] = "171.67.215.200"; // stanford.edu - cali
    ip[2] = "163.195.1.225"; // gov.za - south africa
    ip[3] = "157.7.107.95"; // japan.jp - tokyo
    ip[4] = "43.245.43.59"; // unimelb.edu.au - oceana australia 
    ip[5] = "104.16.103.50"; // nyse
    ip[6] = "186.192.90.12"; // globo.com - brazil
    ip[7] = "37.187.152.154"; // koora.com - France 
    ip[8] = "95.215.19.22"; // futbollibre.net - Finland
    ip[9] = "184.27.197.106"; // infobae.com - India Mumbai
    FILE *fp = fopen("test.out","wb");
    if (fp < 0) errexit("ERROR: cannot open file",NULL);
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv,&tz);
    int startTime = tv.tv_sec;
    int tempTime;
    struct tm *today;
    today = localtime(&tv.tv_sec);
    char *time = malloc(100); 
    while (counter > 0){
        gettimeofday(&tv,&tz);
        tempTime = tv.tv_sec;
        sprintf(time, "%ld", tv.tv_sec - startTime);
        strcat(response, time);
        for (int i = 0; i < 10; i++){
            strcat(request, "ping -c 30 ");
            strcat(request, ip[i]);
            strcat(request, " | tail -1| awk '{print $4}' | cut -d '/' -f 2 > holder.txt");
            system(request);
            size = fread(buffer, 1, sizeof(buffer), fopen("holder.txt","rw")); 
            strcat(response, " ");
            buffer[size-2] = '\0';
            strcat(response, buffer);
            request[0] = '\0';
            memset(buffer,0x0,size);
        }
        fprintf(fp, "%s\n", response);
        counter--;
        response[0] = '\0';
        sleep(15);
    }
    fclose(fp);
}
