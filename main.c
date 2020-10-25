#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define MAX_CICLISTAS 100

typedef struct C //ciclista
{
	int vel; // velocidade -> qauntos ms ele vai demorar p 1 metro nessa volta
	int lin; //linha da pista
	int col; // coluna da pista
 	int vol; //volta
} C;

typedef struct {
	int d; //largura da pista
	int n; //quantidade de ciclistas
	int n_vol; 
	int** p; // <- do professor
} Pista;

//[ [2, 1, 4, 7, 6], [5, 3], [], [] ]

typedef struct {
	pthread_t tids[MAX_CICLISTAS];
	int ids[MAX_CICLISTAS];
	int arrive[MAX_CICLISTAS];
	int go[MAX_CICLISTAS];
} ThreadHelp;

C* ciclistas;
Pista* pista;
ThreadHelp* th;

void exibePista(Pista* P) {
	for(int j=0;j<10;j++){
		for(int k=0;k<(P->d);k++){
			if (P->p[j][k] == 0) {
				printf("| ");
			}
			else {
				printf("|%d",P->p[j][k]);
			}
		}
		printf("|\n");
	}
}

// vetor de ciclistas 
void* ciclista_thread(void* i) {
	int id = *((int *) i);
	C cic = ciclistas[id-1];
	//printf("[Thread] Sou o ciclista %d e comecei a rodar e estou na pista (%d, %d), minha vel: %d\n", id, cic.lin, cic.col, cic.vel);

	printf("[Thread] %d comecei\n", id);
	while (1) {

		if (pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d] == 0) {
			printf("[Thread] %d me mexi para (%d, %d)\n", id, cic.lin, (pista->d + (cic.col-1)%pista->d)%pista->d);
			pista->p[cic.lin][cic.col] = 0;
			pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d] = id;
			th->arrive[id-1] = 1;
		}
		else {
			int id_de_quem_atrapalha = pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d];
			//printf("[Thread] espera perigosa\n");
			while(th->arrive[id_de_quem_atrapalha-1] == 0) continue;
			if (pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d] == 0) {
				printf("[Thread] %d me mexi para (%d, %d)\n", id, cic.lin, (pista->d + (cic.col-1)%pista->d)%pista->d);
				pista->p[cic.lin][cic.col] = 0;
				pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d] = id;
			}
			else {
				printf("[Thread] %d tinha gente\n", id);
			}
			th->arrive[id-1] = 1;
		}

		printf("[Thread] %d quase acabou\n", id);
		while(th->go[id-1] == 0) continue;
		th->go[id-1] = 0;

		printf("[Thread] %d acabei\n", id);
		break;
	}

	// sortea a velocidade
	// ver se tem o cara na frente
	// ver se esta nas duas ultimas voltas
	// n < 5, alguem pode quebrar
	return NULL;
}

// barreira de sincronização

// runner -> loop
void start_run(Pista* P){
	th = (ThreadHelp*) malloc(sizeof(ThreadHelp));
	printf("[Main] Começando a corrida, uhuul!!\n");
	for (int i = 0; i < P->n; i++) {
		th->ids[i] = i+1;
		th->arrive[i] = 0;
		th->go[i] = 0;
		//printf("[Main] Criei a thread %d\n", th->ids[i]);
		pthread_create(&th->tids[i], NULL, ciclista_thread, (void *)&(th->ids[i]));
	}

	for (int rodada = 0; rodada < 1; rodada++) {
		// Aqui eu espero todos os ciclistas rodarem
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			while (th->arrive[cic_id] == 0) continue;
			printf("[Main] %d chegou no arrive\n", cic_id+1);
			th->arrive[cic_id] = 0;
		}

		// Aqui é permito eles rodarem de novo
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->go[cic_id] = 1;
		}
	}

	for (int i = 0; i < P->n; i++) {
		pthread_join(th->tids[i], NULL);
	}
	printf("---------------------\n");
	exibePista(pista);
}

void montaPista(int d, int n, Pista* P)
{
	int i;
	ciclistas = (C*) malloc(P->n * sizeof(C));

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
		// Olho nele!
		ciclistas[v[i]-1].lin = i%5;
		ciclistas[v[i]-1].col = i/5;
		ciclistas[v[i]-1].vel = 120; // 30km/h em milissegundos.
	}
} 


int main(int argc, char** argv)
{
	srand(time(0));
	pista = (Pista *)malloc(sizeof(Pista));
	pista->d = atoi(argv[1]); //tem q ver se eh ```d n``` ou ```n d```
	pista->n = atoi(argv[2]); 
	pista->n_vol = 0;
	montaPista(pista->d,pista->n,pista);
	exibePista(pista);
	start_run(pista);
}