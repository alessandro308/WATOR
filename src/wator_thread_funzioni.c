/** \file wator_thread_funzioni.c
  \author Alessandro Pagiaro
Si dichiara che il contenuto di questo file e' in ogni sua parte opera
originale dell' autore.  */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <time.h>
#include "wator_thread_funzioni.h"

int file_exist(char* x){
	/*Controlli iniziali*/
	FILE* f = fopen(x, "r");
	if(f == NULL){
		return 0;
	}
	else{
		fclose(f);
		return 1;
	}
}

int diviso_parte_intera_sup(int a, int b){
	int tmp;
	tmp = a/b;
	if(tmp*b != a)
		return tmp + 1;
	else 
		return tmp;
}

int my_exp2(int n){
	int p;
	if(n==0)
		return 1;
	else{
		p = my_exp2(n/2);
		if(n%2 == 0)
			return p*p;
		else
			return p*p*2;
	}
}

pezzo_matrice** split_matrice(planet_t* planet){
	int tmp,tmp1;
	pezzo_matrice** lista;
	
	/*Conto in quante sottomatrici (orizzonali e verticali) considerando il caso
	 * in cui nrow/K = 0 */
	numero_matrici_x = diviso_parte_intera_sup(planet -> nrow, K);
	numero_matrici_y = diviso_parte_intera_sup(planet -> ncol, N);

	if((lista = (pezzo_matrice**) malloc(sizeof(pezzo_matrice*)*numero_matrici_x)) == NULL){
		return NULL;
	}
	for(tmp=0; tmp<numero_matrici_x; tmp++){
		if( (lista[tmp] = (pezzo_matrice*) malloc(sizeof(pezzo_matrice)*numero_matrici_y) ) == NULL ){
			return NULL;
		}
	}
	 
	for(tmp=0; tmp<numero_matrici_x; tmp++){
		for(tmp1=0; tmp1<numero_matrici_y; tmp1++){
			lista[tmp][tmp1].start_row = (tmp*K > planet->nrow) ? tmp*K - (tmp*K % planet->nrow) : tmp*K;
			
			lista[tmp][tmp1].start_col = (tmp1*N > planet->ncol) ? tmp1*N - (tmp*N % planet->ncol) : tmp1*N;
			
			lista[tmp][tmp1].end_row = (lista[tmp][tmp1].start_row + K > planet->nrow) ?  
				planet->nrow - 1
				:
				lista[tmp][tmp1].start_row + K-1;
			
			lista[tmp][tmp1].end_col = (lista[tmp][tmp1].start_col + N > planet->ncol) ?
				planet->ncol - 1
				:
				lista[tmp][tmp1].start_col + N-1;
			lista[tmp][tmp1].assegnata = FALSE;
			#ifndef DEBUGOFF
			printf("lista[%d][%d] _start %d _end %d _C_start %d _C_end %d\n", tmp, tmp1, lista[tmp][tmp1].start_row, lista[tmp][tmp1].end_row, lista[tmp][tmp1].start_col, lista[tmp][tmp1].end_col);
			#endif			
		}
	}
	
	return lista;
}
/*Questa serie di funzioni servono per avviare tutti i vari processi dentro wator
 * RETURN 0 se tutto è andato bene
 * 		1 altrimenti
 * */
int start_visualizer(char* dumpfile){
	pid_t pid;
	
	pid = fork();
	if(pid==-1){
		perror("Cannot fork");
		return pid;
	}
	else if(pid==0){
		if( execl("visualizer","visualizer", dumpfile, NULL) == -1 );{
			perror("Cannot Exec");
			return -1;
		}
		return pid;
	}
	else{
		return pid;
	}
}

int add(pezzo_matrice* pezzo){
	struct lista_pezzi* corrente;
	struct lista_pezzi* precedente;
	pthread_mutex_lock(&lista_mux);
	if(lista_pezzi == NULL){
		lista_pezzi = malloc(sizeof(struct lista_pezzi));
		if(lista_pezzi == NULL)
			return 1;
		lista_pezzi -> next = NULL;
		lista_pezzi -> prec = NULL;
		lista_pezzi -> elem = pezzo;
	}
	else{
		corrente = lista_pezzi;
		while (corrente != NULL){
			precedente = corrente;
			corrente = corrente -> next;
		}
		corrente = malloc(sizeof(struct lista_pezzi));
		if(corrente == NULL)
			return 1;
		precedente -> next = corrente;
		corrente -> prec = precedente;
		corrente -> next = NULL;
		corrente -> elem = pezzo;
	}
	pthread_cond_signal(&lista_vuota);
	pthread_mutex_unlock(&lista_mux);
	return 0;
}

struct lista_pezzi* extract(){
	struct lista_pezzi* estratto;
	pthread_mutex_lock(&lista_mux);
	/*Per chiarezza, lista_pezzi è una variabile globale di tipo "struct lista_pezzi**" */
	while(lista_pezzi == NULL){
		pthread_cond_wait(&lista_vuota, &lista_mux);
		if(termine_elaborazione == TRUE){
			/*Basta, tutta la matrice è stata processata, la funzione
			esce dalla lista wait e termina*/
			return NULL;
		}
	}
	assert(lista_pezzi != NULL);
	estratto = lista_pezzi;
	lista_pezzi = lista_pezzi->next;
	count_worker++;
	pthread_mutex_unlock(&lista_mux);
	return estratto;
}

