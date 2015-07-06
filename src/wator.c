 /** \file wator.c
  * \author Alessandro Pagiaro
  Si dichiara che il contenuto di questo file e' in ogni sua parte opera originale dell' autore.  */

#include "wator.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#define CEL_TO_CHECK 4
#define NORD 0
#define EST 3
#define SUD 2
#define OVEST 1
#define FALSE 0
#define TRUE 1
#include <assert.h>
#define DEBUGOFF

char cell_to_char (cell_t a) { 
	if(a == 0)
		return 'S';
	if(a == 1)
		return 'F';
	if(a == 2)
		return 'W';
	else 
		return '?';
}

int char_to_cell (char c) { 
	if(c=='W' || c=='w'){
		return WATER;
	}
	if(c=='S' || c=='s'){
		return SHARK;
	}
	if(c=='F' || c=='f')
		return FISH;
	else
		return -1;
}


void free_planet (planet_t* p){ 
	unsigned int i, nrow;
	if(p != NULL){
		nrow = p -> nrow;
		for(i=0; i<nrow; i++){
			if(p->w != NULL)
				if((p->w)[i] != NULL)
					free((p -> w)[i]);
			if(p->dtime != NULL)
				if((p->dtime)[i] != NULL)
					free((p -> dtime)[i]);
			if(p->btime != NULL)
				if((p->btime)[i]!=NULL)
					free((p -> btime)[i]);
		}
		if(p->dtime != NULL)
			free(p->dtime);
		if(p->btime != NULL)
			free(p->btime);
		if(p->w != NULL)
			free(p->w);
		free(p);
	}
}

planet_t * new_planet (unsigned int nrow, unsigned int ncol){
	unsigned int i, j;
	planet_t *x;
	if(nrow == 0 || ncol == 0){
		errno=EINVAL;
		return NULL;
	}
	
	x = (planet_t*) malloc(sizeof(planet_t)); 
	if(x == NULL){
		perror("Errore allocazione memoria");
		return NULL;
	}
	
	x -> nrow = nrow;
	x -> ncol = ncol;

	(x -> w) = (cell_t**) malloc(sizeof(cell_t*)*nrow); 
	/*Controlla l'avvenuta allocazione*/
	if(x->w == NULL){
		perror("Errore allocazione memoria");
		free_planet(x);
		return NULL;
	}
	
	/*Alloca e controlla*/
	for(i=0; i<nrow; i++){
		(x->w)[i] = (cell_t*) malloc(sizeof(cell_t)*ncol);
		if((x->w)[i]== NULL){
			perror("Errore allocazione memoria");
			free_planet(x);
			return NULL;
		}
	}
	
	/*Alloca e controlla*/
	x -> btime = malloc(sizeof(int*)*nrow);
	if(x->btime==NULL){
		perror("Errore allocazione memoria");
		free_planet(x);
		return NULL;
	}
	for(i=0; i<nrow; i++){
		((x->btime)[i]=malloc(sizeof(int)*ncol));
		if((x->btime)[i]==NULL){
			perror("Errore allocazione memoria");
			free_planet(x);
			return NULL;
		}
	}

	x -> dtime = malloc(sizeof(int*)*nrow);
	if(x->dtime==NULL){
		perror("Errore allocazione memoria");
		free_planet(x);
		return NULL;
	}
	for(i=0; i<nrow; i++){
		(x->dtime)[i]=malloc(sizeof(int)*ncol);
		if((x->dtime)[i]==NULL){
			perror("Errore allocazione memoria");
			free_planet(x);
			return NULL;
		}
	}
	for(i=0; i<nrow; i++){
		for(j=0; j<ncol; j++){
			(x->w)[i][j] = WATER;
			(x->btime)[i][j]=0;
			(x->dtime)[i][j]=0;
		}
	}

	return x;
}

int print_planet (FILE* f, planet_t* p){ 
	unsigned int i,j;
	if(f == NULL){
		perror("Il FILE* f è NULL");
		errno = EINVAL; 
		return -1;
	}
	
	if(p == NULL){
		errno=ERANGE;
		perror("Pianeta non allocato");
		return -1;
	}
	
	if(p->w == NULL){
		errno=ERANGE;
		perror("Pianeta(Matrice, p->w) non allocato correttamente");
		return -1;
	}
	
	if(p->nrow == 0 || p->ncol == 0){
		errno=ERANGE;
		perror("(Ncol OR Nrow) è nullo");
		return -1;
	}
	
	fprintf(f, "%d\n", p->nrow);
	fprintf(f, "%d\n", p->ncol);
	
	for(i=0;i<p->nrow; i++){
		for(j=0; j<p->ncol-1;j++){
			fprintf(f, "%c ", cell_to_char((p->w)[i][j]));
		}
		fprintf(f, "%c\n", cell_to_char((p->w)[i][j]));
	}

	return 0;
}

planet_t* load_planet (FILE* f){ 
	unsigned int nrow, ncol, i, j, y, tmp_size;
	planet_t* p = NULL;
	char a[CIFRE_MAX];/*Array di scansione cifre ncol/nrow*/
	char* tmp=NULL; /*Stringa di scarto strtol*/

	if(f == NULL){ /*Il parametro passato non punta al file*/
		perror("Il FILE* f è NULL");
		errno = EINVAL;
		return NULL;
	}

	/*Cerco il valore di nrow*/
	fgets(a, CIFRE_MAX, f);
	nrow = strtol(a, &tmp, 10);
	if(strcmp(tmp, "\n")){
		perror("Il file non è correttamente formattato");
		errno=ERANGE;
		return NULL;
	}

	/*Cerco il valore di ncol*/
	fgets(a, CIFRE_MAX, f);
	ncol = strtol(a, &tmp, 10);
	if(strcmp(tmp, "\n")){
		perror("Il file non è correttamente formattato");
		errno = ERANGE;
		return NULL;
	}

	tmp_size = ncol*2+1;
	tmp = malloc(sizeof(char)*tmp_size); /*Stringa di appoggio per fgets*/
	/*Creo il pianeta*/
	p = new_planet(nrow, ncol);

	/*Cerco i valori da inserire nella matrice*/
	for(i=0; i<nrow; i++){
		y = 0;
		if(fgets(tmp, tmp_size, f)!= NULL){ /*Prendo la prima riga*/

			/*Inizio controlli di formattazione*/
			for(j=1; j<2*ncol-1; j+=2){
				if(tmp[j] != ' '){
					perror("Il file non è correttamente formattato");
					errno = ERANGE;
					free_planet(p);
					free(tmp);
					return NULL;
				}
			}
			if(tmp[2*ncol-1] != '\n' && i!=nrow-1){
				perror("Il file non è correttamente formattato");
				errno = ERANGE;
				free(tmp);
				free_planet(p);
				return NULL;
			}
			/*Fine controlli di formattazione*/

			for(j=0; j<ncol; j++){ 
				p->w[i][j]=char_to_cell(tmp[y]);
				y+=2;
			}
		}
	}
	free(tmp);
	return p;
}

void free_wator(wator_t* pw){
	if(pw != NULL){
		free_planet(pw -> plan);
		free(pw);
	}
}

wator_t* new_wator (char* fileplan){
	FILE* f = NULL;
	FILE* fconf = NULL;
	int riga=0;
	char a[10];
	/*int num;*/
	char field[3];
	char* scarto;
	
	/*Alloco struttura*/
	wator_t* x = (wator_t*) malloc(sizeof(wator_t));
	if(x == NULL){
		perror("Errore allocazione memoria");
		errno = ERANGE;
		return NULL;
	}

	/*Caricamento wator.conf*/

	fconf=fopen(CONFIGURATION_FILE, "r");
	
	if(fconf == NULL) {
		perror("File wator.conf non aperto");
		free_wator(x);
	  	errno = EINVAL;
	  	return NULL;
	}

	for(riga=0; riga<3; riga++){
		if(fgets(a, 10, fconf)==NULL){
			errno = EINVAL;
			perror("wator.conf è mal formattato");
			fclose(fconf);
			return NULL;
		}
		/*Leggo la stringa e ne ricopio di risultati*/
		/*num = (int) strtol(a+3, &scarto, 10);*/
		field[0]=a[0];
		field[1]=a[1];
		field[2]='\0';

		/*Controllo formattazione*/
		if(a[2]!=' '){
			free_wator(x);
			fclose(fconf);
			errno = EINVAL;
			perror("wator.conf è mal formattato");
			return NULL;
		}
		if(!strcmp(field, "sd") && !strcmp(field, "sb") && !strcmp(field, "fb")){
			free_wator(x);
			fclose(fconf);
			errno = EINVAL;
			perror("wator.conf è mal formattato");
			return NULL;
		}
		/*Fine controllo formattazione */

		if(!strcmp(field,"sd") && riga == 0){
			x->sd = strtol(a+2, &scarto, 10); 
			if(strcmp(scarto, "\n")){
				free_wator(x);
				fclose(fconf);
				errno = EINVAL;
				perror("wator.conf è mal formattato");
				return NULL;
			}
		}
		else if(riga != 1 && riga != 2){
			free_wator(x);
			fclose(fconf);
			errno = EINVAL;
			perror("wator.conf è mal formattato");
			return NULL;
		}

		if(!strcmp(field, "sb") && riga == 1){
			x->sb = strtol(a+2, &scarto, 10);
			if(strcmp(scarto, "\n")){
				free_wator(x);
				fclose(fconf);
				errno = EINVAL;
				perror("wator.conf è mal formattato");
				return NULL;
			}
		}
		else if(riga != 0 && riga != 2){
			free_wator(x);
			fclose(fconf);
			errno = EINVAL;
			perror("wator.conf è mal formattato");
			return NULL;
		}

		if(!strcmp(field, "fb") && riga == 2){
			x->fb = strtol(a+2, &scarto, 10);
			if(strcmp(scarto, "\n")){  
				free_wator(x);
				fclose(fconf);
				errno = EINVAL;
				perror("wator.conf è mal formattato");
				return NULL;
			}
		}
		else if(riga != 1 && riga != 0){
			free_wator(x);
			fclose(fconf);
			errno = EINVAL;
			perror("wator.conf è mal formattato");
			return NULL;
		}
	}
	/*Controllo formattazione file*/
	if(fgets(a, 10, fconf)!=NULL){
		free_wator(x);
		fclose(fconf);
		errno = EINVAL;
		perror("wator.conf è mal formattato");
		return NULL;
	}
	/*Fine Controllo Formattazione file*/
	/*Fine caricamento wator.conf*/
	
	/*Caricamento pianeta*/
	f = fopen(fileplan, "r");
	if(f==NULL){
		free_wator(x);
		fclose(fconf);
		fclose(f);
		errno = EINVAL;
		perror("Errore apertura file");
		return NULL; 
	}

	x->plan = load_planet(f);
	if(x->plan == NULL){
		free_wator(x);
		fclose(fconf);
		fclose(f);
		errno = EINVAL;
		perror("Il pianeta non è stato allocato");
		return NULL; 
	}

	x->nf=fish_count(x->plan);
	x->ns=shark_count(x->plan);
	
	fclose(f);
	fclose(fconf);
	return x;
}

int shark_rule1 (wator_t* pw, int x, int y, int *k, int* l){
	int ncol, nrow, i, mangiato;
	int a[4];
	if((pw==NULL) || (pw->plan == NULL) || (pw->plan->w == NULL) || (pw->plan->dtime == NULL)){
		perror("La struttura non è correttamente allocata");
		errno = EINVAL;
		return -1;
	}
	if((pw->plan->w)[x][y] != SHARK){
		errno = EINVAL;
		return -1;
	}
	
	ncol = pw->plan->ncol;
	nrow = pw->plan->nrow;
	mangiato = 0;
	/*Riempi un array di 4 celle con i numeri [0,1,2,3] casualmente senza ripetizioni*/
	
	a[0] = rand_r(&seed)%CEL_TO_CHECK;
	i = rand_r(&seed)%CEL_TO_CHECK;
	a[1] = (i==a[0]) ? (i+1)%CEL_TO_CHECK : i;
	i = rand_r(&seed)%CEL_TO_CHECK;
	while(i == a[0] || i == a[1]){
		i =(i+1) %CEL_TO_CHECK;
	}
	a[2] = i;
	i=rand_r(&seed)%CEL_TO_CHECK;
	while(i == a[2] || i == a[1] || i == a[0])
		i = (i+1)%CEL_TO_CHECK;
	a[CEL_TO_CHECK-1]=i;
	
	/*Cerca i FISH */
	for(i=0; i<CEL_TO_CHECK && !mangiato; i++){
		if(a[i]==NORD){
			if((pw->plan->w)[(x+(nrow-1))%nrow][y] == FISH){
				mangiato = 1;
				(pw->plan->w)[(x+(nrow-1))%nrow][y] = SHARK;
				(pw->plan->w)[x][y]= WATER;
				(pw->plan->dtime)[(x+(nrow-1))%nrow][y]=(pw->plan->dtime)[x][y];
				(pw->plan->btime)[(x+(nrow-1))%nrow][y] =(pw->plan->btime)[x][y];
				*k = (x+(nrow-1))%nrow;
				*l = y;
				return EAT;
			}
		}
		else if(a[i]==OVEST){
			if((pw->plan->w)[x][(y+(ncol-1))%ncol] == FISH){
				mangiato = 1;
				(pw->plan->w)[x][(y+(ncol-1))%ncol] = SHARK;
				(pw->plan->w)[x][y]= WATER;
				(pw->plan->dtime)[x][(y+(ncol-1))%ncol]=(pw->plan->dtime)[x][y];
				(pw->plan->btime)[x][(y+(ncol-1))%ncol] = (pw->plan->btime)[x][y];
				*k = x;
				*l = (y+(ncol-1))%ncol;
				return EAT;
			}
		}
		else if(a[i]==SUD){
			if((pw->plan->w)[(x+1)%(pw->plan->nrow)][y] == FISH){
				mangiato = 1;
				(pw->plan->w)[(x+1)%(pw->plan->nrow)][y] = SHARK;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->dtime)[(x+1)%(pw->plan->nrow)][y]=(pw->plan->dtime)[x][y];
				(pw->plan->btime)[(x+1)%(pw->plan->nrow)][y] = (pw->plan->btime)[x][y];
				*k = (x+1)%(pw->plan->nrow);
				*l = y;
				return EAT;
			}
		}
		else{ 
			if((pw->plan->w)[x][(y+1)%(pw->plan->ncol)] == FISH){
				mangiato = 1;
				(pw->plan->w)[x][(y+1)%(pw->plan->ncol)] = SHARK;
				(pw->plan->w)[x][y]=WATER;
				(pw->plan->dtime)[x][(y+1)%(pw->plan->ncol)]=(pw->plan->dtime)[x][y];
				(pw->plan->btime)[x][(y+1)%(pw->plan->ncol)] = (pw->plan->btime)[x][y];
				*k = x;
				*l = (y+1)%(pw->plan->ncol);
				return EAT;
			}
		}
	}
	
	/*Ok, non hai mangiato, proviamo a spostarci*/
	for(i=0; i<4; i++){
		if(a[i]==NORD){
			if((pw->plan->w)[(x+(nrow-1))%nrow][y] == WATER){
				(pw->plan->w)[(x+(nrow-1))%nrow][y] = SHARK;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->dtime)[(x+(nrow-1))%nrow][y] = (pw->plan->dtime)[x][y];
				(pw->plan->btime)[(x+(nrow-1))%nrow][y] = (pw->plan->btime)[x][y];
				*k = (x+(nrow-1))%nrow;
				*l = y;
				return MOVE;
			}
		}
		else if(a[i]==OVEST){
			if((pw->plan->w)[x][(y+(ncol-1))%ncol] == WATER){
				(pw->plan->w)[x][(y+(ncol-1))%ncol] = SHARK;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->dtime)[x][(y+(ncol-1))%ncol] = (pw->plan->dtime)[x][y];
				(pw->plan->btime)[x][(y+(ncol-1))%ncol] = (pw->plan->btime)[x][y];
				*k = x;
				*l = (y+(ncol-1))%ncol;
				return MOVE;
			}
		}
		else if(a[i]==SUD){
			if((pw->plan->w)[(x+1)%(pw->plan->nrow)][y] == WATER){
				(pw->plan->w)[(x+1)%(pw->plan->nrow)][y] = SHARK;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->dtime)[(x+1)%(pw->plan->nrow)][y] = (pw->plan->dtime)[x][y];
				(pw->plan->btime)[(x+1)%(pw->plan->nrow)][y] = (pw->plan->btime)[x][y];
				*k = (x+1)%(pw->plan->nrow);
				*l = y;
				return MOVE;
			}
		}
		else /*a[i]==3*/
			if((pw->plan->w)[x][(y+1)%(pw->plan->ncol)] == WATER){
				(pw->plan->w)[x][(y+1)%(pw->plan->ncol)] = SHARK;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->dtime)[x][(y+1)%(pw->plan->ncol)] = (pw->plan->dtime)[x][y];
				(pw->plan->btime)[x][(y+1)%(pw->plan->ncol)] = (pw->plan->btime)[x][y];
				*k = x;
				*l = (y+1)%(pw->plan->ncol);
				return MOVE;
			}
	}
	
	/*Non ha ne' mangiato ne' si e' potuto muovere*/
	return STOP;
}

int shark_rule2 (wator_t* pw, int x, int y, int *k, int* l){
	int i, ncol, nrow;
	int giaFigliato = 0;
	int a[4];
	
	if((pw==NULL) || (pw->plan == NULL) || (pw->plan->w == NULL) || (pw->plan->dtime == NULL)){
		errno = EINVAL;
		return -1;
	}
	*k = x;
	*l = y;
	ncol = pw->plan->ncol;
	nrow = pw->plan->nrow;
	
	/*NASCITA NUOVO SQUALO*/
	if((pw->plan->btime)[x][y] < pw->sb){
		(pw->plan->btime)[x][y] += 1;
	}
	else{ /*Tenta di riprodursi*/
		(pw->plan->btime)[x][y] = 0;
		
		/*Riempi un array di 4 celle con i numeri [0,1,2,3] casualmente senza ripetizioni*/	
		a[0] = rand_r(&seed)%CEL_TO_CHECK;
		i = rand_r(&seed)%CEL_TO_CHECK;
		a[1] = (i==a[0]) ? (i+1)%CEL_TO_CHECK : i;
		i = rand_r(&seed)%CEL_TO_CHECK;
		while(i == a[0] || i == a[1]){
			i =(i+1) %CEL_TO_CHECK;
		}
		a[2] = i;
		i=rand_r(&seed)%CEL_TO_CHECK;
		while(i == a[2] || i == a[1] || i == a[0])
			i = (i+1)%CEL_TO_CHECK;
		a[CEL_TO_CHECK-1]=i;

		/*Cerco posti vuoti dove mettere il figlio*/
		for(i=0; (i<CEL_TO_CHECK && !giaFigliato); i++){
			if(a[i]==NORD){
				if((pw->plan->w)[(x+(nrow-1))%nrow][y] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[(x+(nrow-1))%nrow][y] = SHARK;
					(pw->plan->btime)[(x+(nrow-1))%nrow][y] = 0;
					(pw->plan->dtime)[(x+(nrow-1))%nrow][y] = 0;
					*k = (x+(nrow-1))%nrow;
					*l = y;
					(pw->plan->btime)[x][y]=0;
				}
			}
			else if(a[i]==OVEST){
				if((pw->plan->w)[x][(y+(ncol-1))%ncol] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[x][(y+(ncol-1))%ncol] = SHARK;
					(pw->plan->btime)[x][(y+(ncol-1))%ncol] = 0;
					(pw->plan->dtime)[x][(y+(ncol-1))%ncol] = 0;
					*k = x;
					*l = (y+(ncol-1))%ncol;
					(pw->plan->btime)[x][y]=0;
				}
			}
			else if(a[i]==SUD){
				if((pw->plan->w)[(x+1)%(pw->plan->nrow)][y] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[(x+1)%(pw->plan->nrow)][y] = SHARK;
					(pw->plan->btime)[(x+1)%(pw->plan->nrow)][y] = 0;
					(pw->plan->dtime)[(x+1)%(pw->plan->nrow)][y] = 0;
					*k = (x+1)%(pw->plan->nrow);
					*l = y;
					(pw->plan->btime)[x][y]=0;
				}
			}
			else if(a[i]==EST){
				if((pw->plan->w)[x][(y+1)%(pw->plan->ncol)] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[x][(y+1)%(pw->plan->ncol)] = SHARK;
					(pw->plan->btime)[x][(y+1)%(pw->plan->ncol)] = 0;
					(pw->plan->dtime)[x][(y+1)%(pw->plan->ncol)] = 0;
					*k = x;
					*l = (y+1)%(pw->plan->ncol);
					(pw->plan->btime)[x][y]=0;
				}
			}
		}
	}
	
	/*MORTE SQUALO*/
	if((pw->plan->dtime)[x][y] < pw->sd){
		(pw->plan->dtime)[x][y] += 1;
		return ALIVE;
	}
	else{
		(pw->plan->w)[x][y] = WATER;
		return DEAD;
	}
}

int fish_rule3 (wator_t* pw, int x, int y, int *k, int* l){
	int i, ncol, nrow;
	int a[4];
	int spostato = 0;
	
	if((pw==NULL) || (pw->plan == NULL) || (pw->plan->w == NULL) || (pw->plan->dtime == NULL)){
		errno = EINVAL;
		perror("La struttura non è correttamente allocata");
		return -1;
	}
	if((pw->plan->w)[x][y] != FISH){
		errno = EINVAL;
		perror("FR3 - La cella non contiene un pesce");
		return -1;
	}
	
	ncol = pw->plan->ncol;
	nrow = pw->plan->nrow;
	/*Inizializzo correttamente k, l*/
	*k = x;
	*l = y;
	
	/*Riempi un array di 4 celle con i numeri [0,1,2,3] casualmente senza ripetizioni*/
	a[0] = rand_r(&seed)%CEL_TO_CHECK;
	i = rand_r(&seed)%CEL_TO_CHECK;
	a[1] = (i==a[0]) ? (i+1)%CEL_TO_CHECK : i;
	i = rand_r(&seed)%CEL_TO_CHECK;
	while(i == a[0] || i == a[1]){
		i =(i+1) %CEL_TO_CHECK;
	}
	a[2] = i;
	i=rand_r(&seed)%CEL_TO_CHECK;
	while(i == a[2] || i == a[1] || i == a[0])
		i = (i+1)%CEL_TO_CHECK;
	a[CEL_TO_CHECK-1]=i;
	
	/*Cerco caselle per spostarmi*/
	for(i=0; (i<CEL_TO_CHECK && !spostato); i++){
		if(a[i]==NORD){
			if((pw->plan->w)[(x+(nrow-1))%nrow][y] == WATER){
				(pw->plan->w)[(x+(nrow-1))%nrow][y] = FISH;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->btime)[(x+(nrow-1))%nrow][y] = (pw->plan->btime)[x][y];
				*k = (x+(nrow-1))%nrow;
				*l = y;
				spostato = 1;
				return MOVE;
			}
		}
		else if(a[i]==OVEST){
			if((pw->plan->w)[x][(y+(ncol-1))%ncol] == WATER){
				(pw->plan->w)[x][(y+(ncol-1))%ncol] = FISH;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->btime)[x][(y+(ncol-1))%ncol] = (pw->plan->btime)[x][y];
				*k = x;
				*l = (y+(ncol-1))%ncol;
				spostato = 1;
				return MOVE;
			}
		}
		else if(a[i]==SUD){
			if((pw->plan->w)[(x+1)%(pw->plan->nrow)][y] == WATER){
				(pw->plan->w)[(x+1)%(pw->plan->nrow)][y] = FISH;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->btime)[(x+1)%(pw->plan->nrow)][y] = (pw->plan->btime)[x][y];
				*k = (x+1)%(pw->plan->nrow);
				*l = y;
				spostato = 1;
				return MOVE;
			}
		}
		else 
			if((pw->plan->w)[x][(y+1)%(pw->plan->ncol)] == WATER){
				(pw->plan->w)[x][(y+1)%(pw->plan->ncol)] = FISH;
				(pw->plan->w)[x][y] = WATER;
				(pw->plan->btime)[x][(y+1)%(pw->plan->ncol)] = (pw->plan->btime)[x][y];
				*k = x;
				*l = (y+1)%(pw->plan->ncol);
				spostato = 1;
				return MOVE;
			}
	}
	
	/*Non aveva spazio dove andare*/
	return STOP;
}

int fish_rule4 (wator_t* pw, int x, int y, int *k, int* l){
	int i, ncol, nrow;
	int giaFigliato = 0;
	int a[4];
	
	if((pw==NULL) || (pw->plan == NULL) || (pw->plan->w == NULL) || (pw->plan->dtime == NULL)){
		errno = EINVAL;
		perror("La struttura non è correttamente allocata");
		return -1;
	}
	if((pw->plan->w)[x][y] != FISH){
		errno = EINVAL;
		perror("FR3 - La cella non contiene un pesce");
		return -1;
	}	
	
	nrow = pw->plan->nrow;
	ncol = pw->plan->ncol;
	/*Inizializzo correttamente k, l*/
	*k = x;
	*l = y;
	
	if((pw->plan->btime)[x][y] < pw->fb){
		(pw->plan->btime)[x][y] += 1;
	}
	else{ /*Tenta di riprodursi*/
		(pw->plan->btime)[x][y] = 0;
		
		/*Riempi un array di 4 celle con i numeri [0,1,2,3] casualmente senza ripetizioni*/
		a[0] = rand_r(&seed)%CEL_TO_CHECK;
		i = rand_r(&seed)%CEL_TO_CHECK;
		a[1] = (i==a[0]) ? (i+1)%CEL_TO_CHECK : i;
		i = rand_r(&seed)%CEL_TO_CHECK;
		while(i == a[0] || i == a[1]){
			i =(i+1) %CEL_TO_CHECK;
		}
		a[2] = i;
		i=rand_r(&seed)%CEL_TO_CHECK;
		while(i == a[2] || i == a[1] || i == a[0])
			i = (i+1)%CEL_TO_CHECK;
		a[CEL_TO_CHECK-1]=i;
		
		/*Cerco posti vuoti dove mettere il figlio*/
		for(i=0; (i<CEL_TO_CHECK && !giaFigliato); i++){
			if(a[i]==NORD){
				if((pw->plan->w)[(x+(nrow-1))%nrow][y] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[(x+(nrow-1))%nrow][y] = FISH;
					(pw->plan->btime)[(x+(nrow-1))%nrow][y] = 0;
					(pw->plan->dtime)[(x+(nrow-1))%nrow][y] = 0;
					*k = (x+(nrow-1))%nrow;
					*l = y;
					return 0;
				}
			}
			else if(a[i]==OVEST){
				if((pw->plan->w)[x][(y+(ncol-1))%ncol] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[x][(y+(ncol-1))%ncol] = FISH;
					(pw->plan->btime)[x][(y+(ncol-1))%ncol] = 0;
					(pw->plan->dtime)[x][(y+(ncol-1))%ncol] = 0;
					*k = x;
					*l = (y+(ncol-1))%ncol;
					return 0;
				}
			}
			else if(a[i]==SUD){
				if((pw->plan->w)[(x+1)%(pw->plan->nrow)][y] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[(x+1)%(pw->plan->nrow)][y] = FISH;
					(pw->plan->btime)[(x+1)%(pw->plan->nrow)][y] = 0;
					(pw->plan->dtime)[(x+1)%(pw->plan->nrow)][y] = 0;
					*k = (x+1)%(pw->plan->nrow);
					*l = y;
					return 0;
				}
			}
			else if(a[i]==EST){
				if((pw->plan->w)[x][(y+1)%(pw->plan->ncol)] == WATER){
					giaFigliato = 1;
					(pw->plan->w)[x][(y+1)%(pw->plan->ncol)] = FISH;
					(pw->plan->btime)[x][(y+1)%(pw->plan->ncol)] = 0;
					(pw->plan->dtime)[x][(y+1)%(pw->plan->ncol)] = 0;
					*k = x;
					*l = (y+1)%(pw->plan->ncol);
					return 0;
				}
			}
		}
	}
	return 0;
}

int fish_count (planet_t* p){
	int i, j;
	int n=0;
	
	if(p == NULL || p->w == NULL){
		errno = EINVAL;
		perror("Pianeta non allocato correttamente");
		return -1;
	}
	for(i=0; i<p->nrow; i++){
		for(j=0; j<p->ncol; j++){
			if((p->w)[i][j]==FISH){
				n++;
			}
		}
	}
	return n;
}

int shark_count (planet_t* p){
	int i, j, n;
	
	if(p == NULL || p->w == NULL){
		errno = EINVAL;
		perror("Pianeta non allocato correttamente");
		return -1;
	}
	
	n=0;
	for(i=0; i<p->nrow; i++){
		for(j=0; j<p->ncol; j++){
			if((p->w)[i][j]==SHARK){
				n++;
			}
		}
	}
	return n;
}

#ifdef DEBUG
void stampapianeta(planet_t * p){
	int i, j;
	
	if(p != NULL){
		for(i = 0; i < p->nrow; i++){
			for(j = 0; j < p->ncol; j++){
				if(cell_to_char(*((*((p->w)+i))+j))=='F')
					printf("\e[44m\x1B[37m\u25AE");
				else if((cell_to_char(*((*((p->w)+i))+j))=='S'))
					printf("\e[44m\e[30m\u25AE");
				else 
					printf("\e[44m ");
			}
			printf("\n");
		}
	}
	printf("\n");
	printf("\x1B[0m Fish: %d, Shark: %d\n", fish_count(p), shark_count(p));
}
void stampaMatrice(int** p, int rig, int col){
	int i, j;
	
	if(p != NULL){
		for(i = 0; i < rig; i++){
			for(j = 0; j < col; j++){
				printf("%d ", p[i][j]);
			}
			printf("\n");
		}
	}
	printf("\n");
}
#endif

int update_wator (wator_t * pw){
	printf("Funzione rimpiazzata dalla funzione update_wator_thread. Usare quella\n");
	
	return 0;
}

int update_wator_thread (wator_t *pw, int r_start, int r_end, int c_start, int c_end){
	int i1, j1, k, l, esito1, esito2, esito3, esito4,tmp_skip;
	k = 0;
	l = 0;
	#ifndef DEBUGOFF
	printf("Inizio update\n");
	#endif
	/*Controllo avvio*/
	if((pw==NULL) || (pw->plan == NULL) || (pw->plan->w == NULL) || (pw->plan->dtime == NULL)) {
		errno = EINVAL;
		perror("La struttura non è correttamente allocata");
		return -1;
	}
	/*Fine controlli*/
	
	for(i1=r_start; i1<=r_end; i1++){
		for(j1=c_start; j1<=c_end; j1++){
			#ifndef DEBUGOFF
			printf("Analizzo una cella\n");
			#endif
			/*GESTIONE CASI CRITICI*/
			if(i1 < r_start+2 || i1 > r_end-2 || j1 < c_start+2 || j1 > c_end-2 ){
				#ifndef DEBUGOFF
				printf("Cella critica %d %d \n", i1, j1);
				#endif
				pthread_mutex_lock(&set_critica);
				pthread_mutex_lock(&set_skip);
					tmp_skip = skip[i1][j1];
				pthread_mutex_unlock(&set_skip);	
			}
			else{
				tmp_skip = skip[i1][j1];
			}
			
			if( tmp_skip == FALSE && (pw->plan->w)[i1][j1] == SHARK ){
				#ifndef DEBUGOFF
				printf("Trovato squalo\n");
				#endif
				esito2 = shark_rule2(pw, i1, j1, &k, &l); /*Regola Figlio -> Vivi/Muori*/
				if(esito2 == -1){
					perror("Errore shark_rule2");
					return -1;
				}
				/*<k,l> = posizione figlio se esiste, <i,j> altrimenti*/
				if(i1<r_start+2 || i1 > r_end - 2 || j1 < c_start + 2 || j1 > c_end-2){
					pthread_mutex_lock(&set_skip);
					skip[k][l] = TRUE; /*Imposta di non aggiornare il figlio*/
					pthread_mutex_unlock(&set_skip);
				}
				else{
					skip[k][l] = TRUE;
				}
				#ifndef DEBUGOFF
				printf("Squalo vive ancora, si sposta e mangia\n");
				#endif
				if(esito2 == ALIVE){
					esito1 = shark_rule1(pw, i1, j1, &k, &l); /*Regola Mangia/Spostati*/
					if(esito1 == -1){
						perror("Err shark_rule1");
						return -1;
					}
					if(esito1 == MOVE || esito1 == EAT){
						if(i1<r_start+2 || i1 > r_end - 2 || j1 < c_start + 2 || j1 > c_end-2){
							pthread_mutex_lock(&set_skip);
							skip[k][l] = TRUE;
							pthread_mutex_unlock(&set_skip);
						}
						else{
							skip[k][l] = TRUE;
						}
					}
					if(esito1 == STOP){
						k = i1;
						l = j1;
					}
				}
			}
			
			else if(tmp_skip == FALSE && (pw->plan->w)[i1][j1] == FISH ){
				#ifndef DEBUGOFF
				printf("Trovato pesce\n");
				#endif
				esito4 = fish_rule4(pw, i1, j1, &k, &l);
				if(esito4 == -1){
					perror("Err fish_rule4");
					return -1;
				}
				if(i1<r_start+2 || i1 > r_end - 2 || j1 < c_start + 2 || j1 > c_end-2){
					pthread_mutex_lock(&set_skip);
					skip[k][l] = TRUE;
					pthread_mutex_unlock(&set_skip);
				}
				else{
					skip[k][l] = TRUE;
				}
				#ifndef DEBUGOFF
				printf("Eseguita regola 4 pesce\n");
				#endif
				esito3 = fish_rule3(pw, i1, j1, &k, &l);
				if(esito3 == -1){
					perror("Err fish_rule3");
					return -1;
				}
				if(i1<r_start+2 || i1 > r_end -2 || j1 < c_start + 2 || j1 > c_end-2){
					pthread_mutex_lock(&set_skip);
					skip[k][l] = TRUE;
					pthread_mutex_unlock(&set_skip);
				}
				else{
					skip[k][l] = TRUE;
				}
				#ifndef DEBUGOFF
				printf("Eseguita regola 3 pesce\n");
				#endif
			}

			/*Finiti gli i calcoli sulla cella, ripristino stato matrice critica, se ero in
			un caso critico*/
			if(i1 < r_start+2 || i1 > r_end-2 || j1 < c_start+2 || j1 > c_end-2 ){
				#ifndef DEBUGOFF
				printf("Rilascio mutex\n");
				#endif
				pthread_mutex_unlock(&set_critica);
			}
			#ifndef DEBUGOFF
			printf("Concluso lavori su cella\n");
			#endif
		}
	}
	return 0;
}


