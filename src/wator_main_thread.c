/** \file wator_main_thread.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */
#include "wator.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "wator_thread_funzioni.h"
#include <sys/stat.h> /* per mkdir()*/
#include "valutazione_argomenti.h"

#define NWORK_DEF 4
#define CHRON_DEF 2
#define MAX_TRY 3


/*Dichiaro variabili globali che conterranno tutti i thread id per usarle
 * per la terminazione
 * */
pthread_t* worker_thr_ids;
int num_worker;
pthread_t dispatcher_id, collector_id, root_thread, gestore_segnali_id;
pid_t visualizer;
int close_all;
int termina;
wator_t* simulazione;
char dumpfile[40];
int sig_sigusr1;
struct sigaction sa;	
int puoi_stampare;
int puoi_procedere;

/*Questo è il main del processo principale, WATOR.
 * 1- Controllo di esistenza file
 * 2- Predispone i thread per la gestione corretta dei segnali
 * 3- Avvia i thread
 * 4- Aspetta che terminano il lavoro
 * */
int main (int argc, char *argv[]){
	int n_tentativo;
	wat_proc_conf* x;
	/*Inizializzazione*/
	root_thread = pthread_self();
	/*Inizializza variabili globali di gestione dei segnali*/
	close_all = 0;
	sig_sigusr1 = 0;
	puoi_stampare = 0;
	count_worker = 0;
	puoi_procedere = 1;
	n_tentativo = 0;
	pthread_mutex_init(&set_critica, NULL);
	pthread_mutex_init(&set_skip, NULL);
	pthread_mutex_init(&lista_mux, NULL);
	pthread_mutex_init(&stampa_mux, NULL);
	pthread_cond_init(&lista_vuota, NULL);
	pthread_cond_init(&stanno_lavorando, NULL);
	pthread_cond_init(&stampa_pronta, NULL);	
	pthread_cond_init(&stampa_effettuata, NULL);
	
	if ( mkdir("./tmp", S_IRWXU) ){
		if(errno != EEXIST)
			perror("Errore Creazione Directory");
	}
	/*Rimuovo la socket se esiste, cioè se una precedente esecuzione è stata terminata male*/
	system("rm -f ./tmp/*");
	
	if( file_exist(SOCKNAME) ){
		printf("Il file socket vecchio esiste ancora\n");
		if( remove(SOCKNAME) ){
			perror("Non è stato possibile cancellare la vecchia socket");
			return 1;
		}	
	}
	/*Gestione dei segnali
	* Vengono mascherati i segnali tipici di questo programma, cioè
	* SIGINT, SIGTERM e SIGUSR1
	* L'idea è quella di mascherare i segnali a tutti i thread tranne che 
	* al gestore dei segnali, che si preoccuperà di gestire tutto.
	*/
	sa.sa_handler=SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGINT);
	sigaddset(&sa.sa_mask, SIGUSR2);
	sigaddset(&sa.sa_mask, SIGTERM);
	sigaddset(&sa.sa_mask, SIGUSR1);
	/*Applico maschera a questo thread. La maschera poi viene ereditata dai figli*/
	pthread_sigmask(SIG_BLOCK, &sa.sa_mask, NULL);
	/*Fine Gestione dei segnali*/
	
	/*Carico gli argomenti passati al programma*/
	x = valutazione_argomenti(argc, argv, NWORK_DEF, CHRON_DEF);
	if(x == NULL){
		return 1;
	}
	
	/*Carico il nuovo pianeta*/
	if(!file_exist(x->file)){
		errno = ENOENT;
		perror("File planet");
		return 1;
	}
	simulazione = new_wator(x->file);
	if(simulazione == NULL){
		perror("Errore creazione nuova simulazione");
		return 1;
	}
	
	/*Se non sono stati passati argomenti, assegno quelli di default*/
	if( ( simulazione -> nwork = x->nwork ) == 0)
		simulazione -> nwork = NWORK_DEF;
	if( ( simulazione -> chronon = x->chron ) == 0)
		simulazione -> chronon = CHRON_DEF;	
	if( x->dumpfile == NULL ){
		strcpy(dumpfile,"stdout");
	}
	else{
		strcpy(dumpfile,x->dumpfile);
	}
	intervallo_di_stampa = (x->chron) ? x->chron : CHRON_DEF;
	
	free(x->file);
	free(x->dumpfile);
	free(x);
	
	/*Creo divisione della matrici per far lavorare i thread, funzione delegata dal dispatcher*/
	lista = split_matrice(simulazione->plan);
	/*Avvio Gestore Segnali*/
	n_tentativo=0;
	while( start_gestore_segnali() ){
		if(n_tentativo < MAX_TRY){
			sleep(my_exp2(n_tentativo));
			n_tentativo++;
		}
		else{
			perror("Errore creazione Thread Gestore Segnali");
			return 1;
		}
	}
	
	/*Avvio n worker*/
	n_tentativo=0;
	while ( (worker_thr_ids = start_nworker(simulazione->nwork)) == NULL ){
		if(n_tentativo < MAX_TRY){
			sleep(my_exp2(n_tentativo));
			n_tentativo++;
		}
		else{
			perror("Errore creazione Threads NWorker");
			return 1;
		}
	}
	/*Avvio visualizer*/
	n_tentativo=0;
	while ( (visualizer = start_visualizer(dumpfile)) == -1 ){
		if(n_tentativo < MAX_TRY){
			sleep(my_exp2(n_tentativo));
			n_tentativo++;
		}
		else{
			perror("Errore Start_Visualizer");
			return 1;
		}
	}
	
	/*Avvio thread collector*/
	n_tentativo=0;
	while( start_collector() ){
		if(n_tentativo < MAX_TRY){
			sleep(my_exp2(n_tentativo));
			n_tentativo++;
		}
		else{
			perror("Errore creazione Thread Collector");
			return 1;
		}
	}
	
	/*Avvio Thread Dispacer */
	n_tentativo=0;
	while( start_dispatcher() ){
		if(n_tentativo < MAX_TRY){
			sleep(my_exp2(n_tentativo));
			n_tentativo++;
		}
		else{
			perror("Errore Creazione Thread Dispatcher");
			return 1;
		}
	}
	
	/*Aspetto che i thread lanciati terminano
	*/
	pthread_join(gestore_segnali_id, NULL);
	pthread_join(dispatcher_id, NULL);
	/*Procedura di recupero di collector se dormiva mentre tutti si chiudevano*/
	pthread_mutex_lock(&stampa_mux);
	puoi_stampare = TRUE;
	pthread_cond_broadcast(&stampa_pronta);
	pthread_mutex_unlock(&stampa_mux);
	pthread_join(collector_id, NULL);

	free_wator(simulazione);
	free(worker_thr_ids);

	if( remove(SOCKNAME) ){
		perror("Non è stato possibile rimuovere il file SOCKNAME");
		return 1;
	};
	
	pthread_mutex_destroy(&set_critica);
	pthread_mutex_destroy(&set_skip);
	pthread_mutex_destroy(&lista_mux);
	pthread_mutex_destroy(&stampa_mux);
	pthread_cond_destroy(&lista_vuota);
	pthread_cond_destroy(&stanno_lavorando);
	pthread_cond_destroy(&stampa_pronta);	
	pthread_cond_destroy(&stampa_effettuata);

	return 0;
}
