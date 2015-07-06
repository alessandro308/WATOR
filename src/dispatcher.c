/** \file dispatcher.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */

#include "wator_thread_funzioni.h"
#include <assert.h>
/*Funzione che contiene il codice computazionale del thread dispatcher
 * Invocata alla creazione del Thread
 * DISPATCHER: distribuisce la matrice ai vari thread worker
 * */
static void* dispatcher (void* arg){
	int i, j, chronon;
	FILE* check;
	#ifndef DEBUGOFF
	printf("DISP: Avvio\n");
	#endif
	
	/*Alloca la matrice skip*/
	skip = (int**) malloc(sizeof(int*)*(simulazione->plan->nrow));
	if(skip==NULL){
		perror("Errore di allocazione");
		return NULL;
	}
	for(i=0; i<(simulazione->plan->nrow); i++){
		skip[i]=(int*) malloc(sizeof(int)*(simulazione->plan->ncol));
		if(skip[i]==NULL){
			perror("Errore di allocazione");
			return NULL;
		}
	}
	
	/*Inizio simulazione*/
	chronon = 0;
	/*Close_all è la variabile che viene settata dal Gestore dei Segnali per indicare 
	 * che è stata richiesta la terminazione gentile del processo*/
	while(close_all != TRUE){
		
		/*Azzera matrice Skip per nuovo ciclo*/
		for(i=0; i<simulazione->plan->nrow; i++){
			for(j=0; j<simulazione->plan->ncol; j++){
				skip[i][j] = FALSE;
			}
		}
		
		/*Aspetta che la stampa sia conclusa*/
		pthread_mutex_lock(&stampa_mux);
		/*puoi_procedere è il "segnale" di ok che viene settata dal collector, indica che la
		matrice è stata inviata scritta sulla socket e quindi la matrice può essere modificata*/
		while(!puoi_procedere){
			pthread_cond_wait(&stampa_effettuata, &stampa_mux);
		}
		pthread_mutex_unlock(&stampa_mux);
		
		/*Schedula tutti i task aggiungendo tutti i vari quadranti della matrice creati con la funzione
		split_matrice e copiandone i riferimenti in una nuova lista: lista_pezzi*/ 
		for(i=0; i<numero_matrici_x; i++){
			for(j=0; j<numero_matrici_y; j++)
				/*La add effettua operazioni in mutua esclusione*/
				if(add(lista[i]+j))
					perror("In Dispatcher: Errore ADD");
		}
		
		/*Aspetto che tutti i task siano stati processati*/
		pthread_mutex_lock(&lista_mux);
			/*Se ci sono worker che stanno lavorando oppure dalla lista non sono stati estratti
			 * tutti gli elementi da processare...*/
			while( count_worker || (lista_pezzi != NULL) ){
				pthread_cond_wait(&stanno_lavorando, &lista_mux);
			}
		pthread_mutex_unlock(&lista_mux);
		
		chronon++;
		
		/*Stampo ogni intervallo_di_stampa chronon*/
		if(chronon % intervallo_di_stampa == 0){
			/*Controllo che thread collector esista, altrimenti si va in deadlock*/
			if( pthread_kill(collector_id, 0) ){
				if(errno == ESRCH){
					close_all = TRUE;
					perror("DISP: chiudi tutto, collector non esiste\n");
				}
			}
			
			/*Da l'ok per la stampa*/
			pthread_mutex_lock(&stampa_mux);
				/*Controllo se occorre stampare wator.check*/
				if(stampa_check_file==TRUE){
					check = fopen("wator.check", "w");
					print_planet(check, simulazione->plan);
					fclose(check);
					stampa_check_file=0;
				}
				puoi_procedere = FALSE;
				puoi_stampare = TRUE;
				/*Risveglia collector nel caso sia in attesa*/
				pthread_cond_signal(&stampa_pronta);
			pthread_mutex_unlock(&stampa_mux);
		}	
	}
	
	/*Questa variabile comunica ai thread Worker che è stata richiesta la terminazione gentile
	 * e che l'ultima elaborazione della matrice è terminata*/
	pthread_mutex_lock(&lista_mux);
	termine_elaborazione = TRUE;
	/*Risveglio il collector e tutti i worker, permettendogli di verificare termine_elaborazione*/
	pthread_cond_signal(&stampa_pronta);
	pthread_cond_broadcast(&lista_vuota);
	pthread_mutex_unlock(&lista_mux);
	
	/*Libero memoria skip (usata dagli update) e la matrice di split_matrice*/
	for(i=0; i<simulazione->plan->nrow; i++){
		free(skip[i]);
	}
	free(skip);
	for(i=0;  i<numero_matrici_x; i++)
		free(lista[i]);
	free(lista);
	return NULL;
}

/*Attivatore del thread dispatcher
 * Viene invocato dal processo wator nel main
 */
int start_dispatcher(){
	int fallito;
	fallito = pthread_create(&dispatcher_id, NULL, &dispatcher, NULL);
	if (fallito){
		perror("Errore nella creazione del Thread Dispacer");
		return -1;
	}
	return 0;
}
