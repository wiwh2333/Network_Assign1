/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 22135

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}



int main(int argc, char **argv) {
  int sockfd; /* socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  //Added by William White
  int valread; /* Used to determine num of elms File reads*/
  int filesize; /*Stores filesize*/
  char filename[BUFSIZE]; /*Stores filename*/


  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      //printf("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      printf("ERROR on inet_ntoa\n");
    //printf("server received datagram from %s (%s)\n", 
	   //hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
    
    
    /* 
     * sendto: echo the input back to the client 
     */
    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");

    //Interperet message
    //GET
    if (strncmp(buf, "get",3) == 0) {
      valread = recvfrom(sockfd, filename, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen); if (valread < 0) {error("ERROR in recv");} //FileName
      FILE *fp = fopen(buf + 4, "rb"); //Open File
      if (fp == NULL){perror("Error opening File");}
      
      valread = fread(buf, 1, BUFSIZE, fp); //Read BUFSIZE from the file or until file end
      n = sendto(sockfd, buf, strlen(buf), 0, &clientaddr, clientlen);//Send buf
      if (n < 0) {error("ERROR in sendto");}
      //printf("Written: %d Recieved: %d\n",n, valread);
      fclose(fp);
    }
    //PUT
    if (strncmp(buf, "put",3) == 0) {
      valread = recvfrom(sockfd, filename, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen); if (valread < 0) {error("ERROR in recv");} //FileName
      // printf("File:%s", filename);
      // for (int i = 0; i<strlen(filename)+1; i++) {
      //         printf("Character: %c, ASCII Value: %d\n", filename[i], filename[i]);
      //       }
      //strcat(filename,'\n');
      //printf("File:%s", filename);
      FILE *fp = fopen(filename, "wb");
      if (fp == NULL){perror("Error opening File");}
      valread = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen); if (valread < 0) {error("ERROR in recv");} //Data
      n = fwrite(buf, 1, valread, fp); //Writes valread bytes of data from buf to fp
      printf("Written: %d Recieved: %d\n",n, valread);
      fclose(fp);
    }
    //DELETE
    if (strncmp(buf, "delete",6) == 0) {
      valread = recvfrom(sockfd, filename, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen); if (valread < 0) {error("ERROR in recv");} //FileName
      if (remove(filename) == 0){printf("File %s deleted",filename);}
      else{ printf("Can't Delete%s",filename);}
    }
    //LS
    if (strncmp(buf, "ls",2) == 0) {
      struct dirent *entries;
      char dir_list[BUFSIZE]=""; 
      DIR *dr = opendir(".");

      if (dr == NULL){ printf("Can't Open");}
      while((entries = readdir(dr)) != NULL){
        strcat(dir_list,entries->d_name);
        strcat(dir_list," ");
      }
      closedir(dr);
      for(int i = 0; i<strlen(dir_list);i++){
        if (dir_list[i] == '\n') {dir_list[i] = ' ';}
      }
      int i =0;
      n = sendto(sockfd, dir_list, strlen(dir_list)+1, 0, &clientaddr, clientlen);//Send buf
      if (n < 0) {error("ERROR in sendto");}
      
    }
    //EXIT
    if (strncmp(buf, "exit",4) == 0) {
            printf("Exiting...\n");
            break;
        }
     
  }
}
