/** \file collector.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */
#include "wator_thread_funzioni.h"
#define TOKEN "A"

static void* collector (void* arg){	
	int fd_skt, i, j;
	struct sockaddr_un sockAdd;
	char string[10];
	planet_t* p;
	strncpy(sockAdd.sun_path, SOCKNAME, UNIX_PATH_MAX); 
	sockAdd.sun_family=AF_UNIX;

	fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd_skt == -1){
		perror("Errore socket client");
	}

	/*Prova a connetterti. Se la socket non esiste, aspetta 0,25 secondi e ritenta*/
	while( connect(fd_skt, (struct sockaddr*) &sockAdd, sizeof(sockAdd)) == -1){
		if( errno == ENOENT ){
			usleep(250000);
		}
		else{
			perror("Errore connessione socket client");
			close_all = TRUE; /*Termina l'intero processo*/
			return NULL;
		}
	}
	p = simulazione->plan;

	/*Inizio ciclo iterativo del thread*/
	while( termine_elaborazione == FALSE ){
		pthread_mutex_lock(&stampa_mux);
		/*Si mette in attesa finchè il dispatcher non comunica la possibilità di stampare*/
		while(!puoi_stampare){
			pthread_cond_wait(&stampa_pronta, &stampa_mux);
		}
		pthread_mutex_unlock(&stampa_mux);
		
		/*Scrive un carattere TOKEN che fa capire al visualizer che è terminata una matrice e ne sta
		per iniziare una nuova*/
		write(fd_skt, TOKEN, 1);
		sprintf(string, "%d\n", p->nrow);
		write(fd_skt, string, strlen(string));
		sprintf(string, "%d\n", p->ncol);
		write(fd_skt, string, strlen(string));
		for(i=0;i<p->nrow; i++){
			for(j=0; j<p->ncol-1;j++){
				sprintf(string, "%c ", cell_to_char((p->w)[i][j]));
				write(fd_skt, string, strlen(string));
			}
			sprintf(string, "%c\n", cell_to_char((p->w)[i][j]));
			write(fd_skt, string, strlen(string));
		}
		
		/*Comunica che è stata effettuata la stampa e quindi il dispatcher può
		 * nuovamente modificare la matrice*/
		pthread_mutex_lock(&stampa_mux);
			puoi_stampare = FALSE;
			puoi_procedere = TRUE;
		pthread_cond_signal(&stampa_effettuata);
		pthread_mutex_unlock(&stampa_mux);
	}
	close(fd_skt);
	return NULL;
}

int start_collector(){
	int fallito;
	fallito = pthread_create(&collector_id, NULL, &collector, NULL);
	if (fallito){
		perror("Errore nella creazione del Thread Collector");
		return -1;
	}
	return 0;
}
