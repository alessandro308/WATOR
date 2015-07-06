/** \file worker.h
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.*/
#include "wator_thread_funzioni.h"

static void* worker (void* arg){ 
	struct lista_pezzi* to_free;
	pezzo_matrice* matrice_da_elaborare;
	int wid;
	FILE* f;
	char stringa[40];
	
	wid = *(int*) arg;
	sprintf(stringa, "wator_worker_%d", wid);
	if( (f = fopen(stringa, "w")) == NULL ){
		perror("Errore Creazione File");
		exit(EXIT_FAILURE);
	}
	else{
		fclose(f);
	}
	
	while(termine_elaborazione == FALSE){
		to_free = extract();
		if(to_free == NULL && termine_elaborazione == TRUE){
			pthread_exit(0);
		}
		matrice_da_elaborare = to_free->elem;
		
		if( update_wator_thread(simulazione,
			(matrice_da_elaborare)->start_row, 
			(matrice_da_elaborare)->end_row, 
			(matrice_da_elaborare)->start_col, 
			(matrice_da_elaborare)->end_col) == -1 )
		{
			perror("Worker, errore aggiornamento");
			close_all = 1;
		}
		
		/*Libero dalla memoria il pezzetto da cui ho estratto il puntatore alla sottomatrice*/
		free(to_free);
		pthread_mutex_lock(&lista_mux);
		/*Terminata l'update rimuovo thread dai count_worker*/
		count_worker--;
		if(lista_pezzi == NULL && !count_worker){
			if(termine_elaborazione == TRUE){ 
				pthread_exit(0);
			}
			/*Sblocco dispatcher avvisandolo che tutta la matrice è stata processata*/
			pthread_cond_signal(&stanno_lavorando);
		}
		pthread_mutex_unlock(&lista_mux);
	}
	free(arg);
	pthread_exit(0);
}

pthread_t* start_nworker(int n){
	pthread_t* t_id;
	int i, fallito;
	t_id = (pthread_t*) malloc(sizeof(pthread_t)*n);

	termine_elaborazione = 0;

	/*Inizializzo variabile seed, usata dalla rand_r nelle funzione di aggiornamento*/
	seed = time(NULL);
	
	/*Creo gli n worker*/
	for(i=0; i<n; i++){
		int *arg = malloc(sizeof(*arg));
		if ( arg == NULL ) {
			perror("Couldn't allocate memory for thread arg.\n");
			return NULL;
		}
		*arg = i;
		fallito = pthread_create(t_id+i, NULL, &worker, (void* ) arg);
		if (fallito){
			free(t_id);
			perror("Errore nella creazione di un Thread Worker");
			return NULL;
		}
	}
	return t_id;
}