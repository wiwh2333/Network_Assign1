/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 22135

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen , valread;
    int filesize;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    bzero(buf, BUFSIZE);
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);

    
    

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");

  //GET COMMAND   
    if (strncmp(buf, "get",3) == 0) {
      //Send "{File Name}"
      n = sendto(sockfd, buf + 4, strlen(buf)-4, 0, &serveraddr, serverlen);
      if (n < 0) {error("ERROR in sendto");}
      //ECHO
      n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
      //Write to File
      FILE *fp = fopen(buf + 4, "wb");
      valread = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &serveraddr, &serverlen); if (valread < 0) {error("ERROR in recv");} //Data
      n = fwrite(buf, 1, valread, fp); //Writes valread bytes of data from buf to fp
      //printf("Written: %d Recieved: %d\n",n, valread);
      fclose(fp);
    }
  //Put Command Recieved
    if (strncmp(buf, "put",3) == 0) {
            //Send "{File Name}"
            n = sendto(sockfd, buf + 4, strlen(buf)-4, 0, &serveraddr, serverlen);
            if (n < 0) {error("ERROR in sendto");}
            //Open {File Name}, and sendto until the file is empty
            FILE *fp = fopen(buf + 4, "rb");
            if (fp == NULL){perror("Error opening File");}
            valread = fread(buf, 1, BUFSIZE, fp);
            sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
            if (n < 0) {error("ERROR in sendto");}
            fclose(fp);
            n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
            if (n < 0) 
              error("ERROR in recvfrom");
            //printf("Echo from server: %s", buf);
        }
  //Delete Command
    if (strncmp(buf, "delete",6) == 0) {
        //Send "{File Name}"
        n = sendto(sockfd, buf + 7, strlen(buf)-7, 0, &serveraddr, serverlen);
        if (n < 0) {error("ERROR in sendto");}
    }
  //LS Command
    if (strncmp(buf, "ls",2) == 0) {
        n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);//ECHO
        n = recvfrom(sockfd, buf, sizeof(buf), 0, &serveraddr, &serverlen);//Message
        if (n < 0) {error("ERROR in recvfrom");}
        printf("List of Files:%s",buf);
    }
    //
    
    /* print the server's reply */
    // n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
    // if (n < 0) 
    //   error("ERROR in recvfrom");
    // printf("Echo from server: %s", buf);
    return 0;
}
