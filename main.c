#include <stdlib.h>
#include <stdio.h>

typedef struct C //ciclista
{
	int vel; // velocidade -> qauntos ms ele vai demorar p 1 metro nessa volta
	int p; //linha da pista
	int vol; //volta
} C;


typedef struct pista
{
	int q; //quantidade de ciclistas
	int f**; //fila de ciclistas
	int p**;
} pista;

int montaPista(int d, int n, pista P)
{
	
	for (int i=0; i<10; i++){
	P->p[i][] = (int *)malloc(10*sizeof(int *));
		for (int j=0; j<d; j++){
		P->p[i][j] = (int)malloc(sizeof(int));
		} 
	}

	for(int k=0;k<n;k++){
		aux[k]=k;
	}
	if (aux[k] != 0)
		P->p[][] = random()%5;
	
}

int main(int argc, char** argv)
{

	pista = argv[1]; //tem q ver se eh ```d n``` ou ```n d```
}

/*	for (int i=0; i<10; i++){
		D[i][] = (int *)malloc(10*sizeof(int *));
		for (int j=0; j<d; j++){
			D[i][j] = (int)malloc(sizeof(int));
		} 
	}
*/
