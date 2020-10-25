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
	int vivo;
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

int cpo; //ciclistas que passaram a origem
int ciclistasVivos;
int voltaAtual = 1;
C* ciclistas;
Pista* pista;
ThreadHelp* th;
pthread_mutex_t mutex;

void exibePista(Pista* P) {
	fprintf(stderr,"volta %d\n",voltaAtual);
	pthread_mutex_lock(&mutex);
	for(int j=0;j<10;j++){
		for(int k=0;k<(P->d);k++){
			if (P->p[j][k] == 0) {
				fprintf(stderr,"| ");
			}
			else {
				fprintf(stderr,"|%d",P->p[j][k]);
			}
		}
		fprintf(stderr,"|\n");
	}
	pthread_mutex_unlock(&mutex);
	fprintf(stderr,"---------------------\n");
}

// vetor de ciclistas 
void* ciclista_thread(void* i) {
	int id = *((int *) i);
	C cic = ciclistas[id-1];
	int proxLin;
	int proxCol;
	//printf("[Thread] Sou o ciclista %d e comecei a rodar e estou na pista (%d, %d), minha vel: %d\n", id, cic.lin, cic.col, cic.vel);

	printf("[Thread] %d comecei\n", id);
	while (1) {
		proxCol=cic.col;
		proxLin=cic.lin;
		pthread_mutex_lock(&mutex);
		int quem=pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d] ;//quem esta na posicao que eu quero ir
		pthread_mutex_unlock(&mutex);
		if (quem == 0) {
			proxCol = (pista->d + (cic.col-1)%pista->d)%pista->d;
			printf("[Thread] %d me mexi para (%d, %d)\n", id, cic.lin, proxCol);
		}
		else {
			printf("[Thread] %d esperando %d terminar sua rodada\n", id, quem);
			while(th->arrive[quem-1] == 0) continue;
			printf("[Thread] %d terminou a espera.\n", id);
			pthread_mutex_lock(&mutex);
			int proxQuem = pista->p[cic.lin][(pista->d + (cic.col-1)%pista->d)%pista->d];
			pthread_mutex_unlock(&mutex);
			if (proxQuem == 0) {
				proxCol = (pista->d + (cic.col-1)%pista->d)%pista->d;
				printf("[Thread] %d me mexi para (%d, %d)\n", id, cic.lin, proxCol);
			}
			else {
				printf("[Thread] %d tinha gente\n", id);
			}
		}
		pthread_mutex_lock(&mutex);
		pista->p[cic.lin][cic.col] = 0;
		pista->p[proxLin][proxCol] = id;
		pthread_mutex_unlock(&mutex);
		cic.lin=proxLin;
		cic.col=proxCol;
		printf("id %d foi liberado\n", id);
		th->arrive[id-1] = 1;
		//OLHO NELE !!!
		if(ciclistasVivos ==1) break;
		
		//ver se sou o ultimo e me destruir caso for
		if(cic.col ==0 && voltaAtual%2==0){
			//locka
			cpo++;
			if(cpo == ciclistasVivos){
				//sou o ultimo
				printf("[morre] %d volta %d\n",id,voltaAtual);
				ciclistas[id-1].vivo = 0;
				pista->p[cic.lin][cic.col] = 0;
				ciclistasVivos--;
				cpo=0;
				break;
			}
		}
		
		while(th->go[id-1] == 0) continue;
		th->go[id-1] = 0;
		
		printf("[Thread] %d acabei\n", id);
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
	exibePista(pista);
	th = (ThreadHelp*) malloc(sizeof(ThreadHelp));
	int i = 0;

	//printf("[Main] Começando a corrida, uhuul!!\n");
	for (int i = 0; i < P->n; i++) {
		th->ids[i] = i+1;
		th->arrive[i] = 0;
		th->go[i] = 0;
		//printf("[Main] Criei a thread %d\n", th->ids[i]);
		pthread_create(&th->tids[i], NULL, ciclista_thread, (void *)&(th->ids[i]));
	}
	while (ciclistasVivos > 1) {
		i++;
		// Aqui eu espero todos os ciclistas rodarem
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			printf("[zumbi] %d ->%d\n",cic_id+1,ciclistas[cic_id].vivo);
			if(ciclistas[cic_id].vivo==0){ 
				printf("%d nao sera executado\n", cic_id);
				continue;
			}
			printf("%d esperando arrive\n",cic_id+1);
			while (th->arrive[cic_id] == 0) continue;
			printf("[Main] %d chegou no arrive\n", cic_id+1);
		}
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->arrive[cic_id] = 0;
		}

		// Aqui é permito eles rodarem de novo
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->go[cic_id] = 1;
		}
		exibePista(pista);
		printf("**************************\n");
		if(i%pista->d == 0)
			//locka?
			voltaAtual = (voltaAtual+1);
	}
	printf("FIM DO WHILE vivos-> %d\n",ciclistasVivos);
	for (int i = 0; i < P->n; i++) {
		pthread_join(th->tids[i], NULL);
	}

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
	ciclistasVivos = n;
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
		ciclistas[i].vivo = 1;
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
	pthread_mutex_init(&mutex, NULL);
	montaPista(pista->d,pista->n,pista);
	//exibePista(pista);
	start_run(pista);
	pthread_mutex_destroy(&mutex);
}
