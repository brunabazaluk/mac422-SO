#include <stdlib.h>
#include <stdio.h>
#include <time.h>

typedef struct C //ciclista
{
	int vel; // velocidade -> qauntos ms ele vai demorar p 1 metro nessa volta
	int p; //linha da pista
	int vol; //volta
} C;

typedef struct pista
{
	int d; //largura da pista
	int q; //quantidade de ciclistas
	int lin;
	int col; 
	int** p;
} pista;

// vetor de ciclistas 

// barreira de sincronização

// threads

// runner -> loop
// sincroniza!

void montaPista(int d, int n, pista* P)
{
	int i;
	P->p = (int **)malloc(10*sizeof(int *));
	for (i=0; i<10; i++){
		P->p[i] = (int *)malloc(d*sizeof(int));
		for (int j=0; j<d; j++){
			P->p[i][j] = 0;
		} 
	}
	int v[n];
	for (i = 0; i < n; i++) {
		v[i] = i + 1;
	}
	int index;
    for (i = 0; i < n; i++) {
		// lower = i
 		index = (rand() % (n-1 - i + 1)) + i;
		int aux = v[i];
		v[i] = v[index];
		v[index] = aux;
	}	
	//int r = n/5;
	for (i = 0; i < n; i++) {
		P->p[i%5][i/5] = v[i];
	}
} 

int main(int argc, char** argv)
{
	srand(time(0));
	pista* P = (pista *)malloc(sizeof(pista));
	P->d = atoi(argv[1]); //tem q ver se eh ```d n``` ou ```n d```
	P->q = atoi(argv[2]); 
	
	montaPista(P->d,P->q,P); 
	for(int j=0;j<10;j++){
		for(int k=0;k<(P->d);k++){
			printf("|%d",P->p[j][k]);
		}
		printf("|\n");
	}

}