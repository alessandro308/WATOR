/** \file wator_thread_funzioni.h
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "wator.h"

#include <assert.h>

#ifdef __APPLE__
#define UNIX_PATH_MAX 104
#endif
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif
#define SOCKNAME "./tmp/mysock"
#define SCK_MAX_CONNECTION 2
#define NS 100
/*Fine Roba Socket*/
#define K 7
#define N 7
#define FALSE 0
#define TRUE !FALSE
#define DEBUGOFF

extern pthread_t collector_id;
extern pthread_t dispatcher_id;
extern pthread_t gestore_segnali_id;
extern char dumpfile[40];
extern int sig_sigusr1;
extern int close_all;
extern wator_t* simulazione;
extern struct sigaction sa;
extern int puoi_stampare;
extern int puoi_procedere;
int count_worker;
int intervallo_di_stampa;
pthread_mutex_t set_skip;
pthread_mutex_t set_critica;
/*Gestione lista pezzi matrici per dispatcher*/
pthread_mutex_t lista_mux;
pthread_cond_t lista_vuota;
/*Comunicazione tra dispatcher e worker*/
pthread_cond_t stanno_lavorando;  
/*Comunicazione tra dispatcher e collector*/
pthread_cond_t stampa_pronta;
pthread_cond_t stampa_effettuata;
pthread_mutex_t stampa_mux;

int stampa_check_file;
int** skip;
int** critica;
int termine_elaborazione;
/*Seme rand_r*/
unsigned int seed;

/*Strutture per la gestione della divisione in matrici*/
typedef struct _pezzo_matrice {
	int start_col;
	int end_col;
	int start_row;
	int end_row;
	int assegnata;
} pezzo_matrice;

/*Variabili usata da split_matrice*/
pezzo_matrice **lista;

struct lista_pezzi{
	struct lista_pezzi* prec;
	struct lista_pezzi* next;
	pezzo_matrice* elem;
};

struct lista_pezzi* lista_pezzi;
int numero_matrici_x;
int numero_matrici_y;
/*PARAM file da verificare
 * RET 1 se il file esiste
 * 		0 se il file non esiste
 */
int file_exist(char* file_name);
int my_exp2(int n);

/*Questa serie di funzioni servono per avviare tutti i vari processi dentro wator
 * RETURN 0 se tutto è andato bene
 * 		1 altrimenti
 * */
pid_t start_visualizer();
int start_dispatcher();
int start_collector();
int start_gestore_segnali();
/* Questa funzione avvia n thread worker
 * PARAM [n]umero di worker che devono essere creati
 * RETURN NULL se qualcosa è andato storto
 * 		un puntatore pthread_t* di dimensione n contenente i thr_id degli n creati
 * */
pthread_t* start_nworker(int n);

/*\RET la parte intera superiore di a/b */
int diviso_parte_intera_sup(int a, int b);

/*Divide la matrice in pezzi di dimensione KxN, dove K,N sono definiti da opportune macro.
 * \RET Se non ci sono stati errori, una lista di elementi contenenti varie informazioni:
 * 		int start_col - Contenente la colonna di inizio sottomatrice
 * 		int end_col - Contenente la colonna di fine sottomatrice, fondamentale se N non è multiplo di ncol
 * 		int start_row - Contentente la riga di inizio sottomatrice
 * 		int end_row - Analogo a end_col
 * 		int chronon_corrente - Variabile settata a 0
 * ritorna NULL altrimenti, settando errno
 */
pezzo_matrice** split_matrice(planet_t* planet);

/*Sostituisce la ADD. Resetta a 0 i pezzi già assegnati e i puntatori a questi*/
int add(pezzo_matrice* elem);
/*Se la lista è vuota, si mette in attesa finchè non si riempie. Incrementa il numero di count_worker.
 * \RET two_int{x,y} indici da usare per accedere a lista_sottomatrice. */
struct lista_pezzi* extract();
