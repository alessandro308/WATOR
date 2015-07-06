/** \file visualizer.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */
/*Roba per socket*/
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#ifdef __APPLE__
#define UNIX_PATH_MAX 104
#endif
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif
#define SOCKNAME "./tmp/mysock"
#define SCK_MAX_CONNECTION 2
#define NS 5

#define TOKEN 'A'
/*Fine Roba Socket*/

int main (int argc,char *argv[]){
	int fd_skt, fd_c, esito_lettura;
	struct sockaddr_un sa;
	char* buf;
	char dumpfile[105];
	int dumpfile_fd;
	char* substring, *temp_buf;
	
	sa.sun_family=AF_UNIX;
	strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
	
	strncpy(dumpfile, argv[1], 104);
	if(!strcmp(dumpfile, "stdout")){
		dumpfile_fd = STDOUT_FILENO;
	}
	else{
		dumpfile_fd = creat(dumpfile, 0755);
		if(dumpfile_fd == -1){
			perror("Errore open dumpfile");
		}
	}
 
	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd_skt == -1){
		perror("Errore creazione Socket");
		return 1;
	}
	if ( bind( fd_skt, (struct sockaddr*) &sa, sizeof(sa) ) == -1 ){
		perror("Errore Bind");
		return 1;
	} ;
	if ( listen(fd_skt, SCK_MAX_CONNECTION) == -1 ){
		perror("Errore listen");
		return 1;
	}
	
	fd_c = accept(fd_skt, NULL, 0);
	if(fd_c == -1){
		perror("Errore Accept");
		return 1;
	}

	buf = (char *)malloc((NS+1)*sizeof(char));
	if (buf == NULL){
		perror("Errori allocazione buf");
		return 1;
	}

	memset(buf, '\0', NS+1);
	
	while ( (esito_lettura=read(fd_c, buf, NS)) != 0){
		if( esito_lettura == -1){/*C'è stato un errore*/	
			perror("Errore read fd_c");
			return 1;
		}
		else if(esito_lettura == 0){
			break;
		}
		else{
			substring = NULL;
			substring = strrchr(buf, TOKEN);
			if(substring != NULL){
				*substring = '\0';
				write(dumpfile_fd, buf, strlen(buf));
				/* Si posiziona all'inizio della nuova stringa */
				temp_buf = substring + 1;
				/* chiude e riapre il file troncandolo */
				if (dumpfile_fd != STDOUT_FILENO){
					if( close(dumpfile_fd) == -1){
						perror("Errore chiusura dumpfile_fd");
					}
					dumpfile_fd = creat(dumpfile, 0755);
				}
				write(dumpfile_fd, temp_buf, strlen(temp_buf));
			} else {
				write(dumpfile_fd, buf, esito_lettura);
			}
			/*Pulisco buf per evitare lettura sporche*/
			memset(buf, '\0', NS+1);
		}
	}

	if (dumpfile_fd != STDOUT_FILENO && close(dumpfile_fd) < 0 ){
		perror("Close dumpfile_fd");
		return 1;
	}
	if ( close(fd_skt) < 0 ){
		perror("Close fd_skt");
		return 1;
	}
	if ( close(fd_c) < 0 ){
		perror("Close fd_c");
		return 1;
	}
	return 0;
}
