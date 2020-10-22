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
	int d; //largura da pista
	int q; //quantidade de ciclistas
	int lin;
	int col; 
	int** p;
} pista;

int montaPista(int d, int n, pista* P)
{
	int i;
	P->p = (int **)malloc(d*sizeof(int *));
	for (i=0; i<d; i++){
		P->p[i] = (int *)malloc(10*sizeof(int));
		//for (int j=0; j<d; j++){
		//P->p[i][j] = (int)malloc(sizeof(int));
		//} 
	}
	printf("debug1");
	int v[n];
	printf("debug2;");
	int index;
    for (i = 0; i < d; i++) {
 		index = (rand() %  (n - 2));
		int aux = v[i];
		v[i]=v[index];
		v[index]=aux;
	}
	//int r = n/5;
	for (i = 0; i < d; i++) {
		P->p[i/5][i%5] = v[i];
	}
} 

int main(int argc, char** argv)
{
	printf("%d ",argc);//,argv[2]);
	pista* P = (pista *)malloc(sizeof(pista));
	P->d = atoi(argv[1]); //tem q ver se eh ```d n``` ou ```n d```
	P->q = atoi(argv[2]); 
	
	printf("debug0");
	montaPista(P->d,P->q,P); 
	for(int j=0;j<(P->d);j++){
		for(int k=0;k<(10);k++){
			printf("|%d|",P->p[j][k]);
		}
		printf("\n");
	}

}

/*	for (i=0; i<10; i++){
		D[i][] = (int *)malloc(10*sizeof(int *));
		for (int j=0; j<d; j++){
			D[i][j] = (int)malloc(sizeof(int));
		} 
	}
*/
