/** \file gestione_segnali.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */

#include "wator_thread_funzioni.h"

static void* gestore_segnali(void* arg){
	int sig_arrived;
	
	while(!close_all){
		sigwait(&sa.sa_mask, &sig_arrived);
		if(sig_arrived == SIGINT || sig_arrived == SIGTERM){
			close_all=1;
			return NULL;
		}
		else if(sig_arrived == SIGUSR1){
			/*Manda il comando di stampare il check_file, funzione svolta dal collector*/
			stampa_check_file = 1;
		}
	}
	return NULL;
}

int start_gestore_segnali(){
	int fallito;
	fallito = pthread_create(&gestore_segnali_id, NULL, &gestore_segnali, NULL);
	if (fallito){
		perror("Errore nella creazione del Thread Gestore Segnali");
		return -1;
	}
	return 0;
}

