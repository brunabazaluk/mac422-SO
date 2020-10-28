#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

#define MAX_CICLISTAS 100

typedef struct C //ciclista
{
	int vel; // velocidade -> qauntos ms ele vai demorar p 1 metro nessa volta
	int lin; //linha da pista
	int col; // coluna da pista
 	int vol; //volta
	int vivo;
	int pos; //posicao
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

int *rank;
int cpo; //ciclistas que passaram a origem
int ciclistasVivos;
int voltaAtual = 1;
int voltaAnterior = 0;
C* ciclistas;
Pista* pista;
ThreadHelp* th;
pthread_mutex_t m_pista;
pthread_mutex_t m_ciclistasVivos;
int ciclistaCorreu;

void exibePista(Pista* P) {
	fprintf(stderr,"volta %d\n",voltaAtual);
	pthread_mutex_lock(&m_pista);
	for(int j=0;j<10;j++){
		for(int k=0;k<(P->d);k++){
			if (P->p[j][k] == 0) {
				fprintf(stderr,"|  ");
			}
			else {
				fprintf(stderr,"|%2d",P->p[j][k]);
			}
		}
		fprintf(stderr,"|\n");
	}
	pthread_mutex_unlock(&m_pista);
	fprintf(stderr,"---------------------------\n");
}


// vetor de ciclistas 
void* ciclista_thread(void* i) {
	int id = *((int *) i);
	C* cic = &ciclistas[id-1];
	int proxLin;
	int proxCol;
	int cv; // ciclistas vivos local
	//printf("[Thread] Sou o ciclista %d e comecei a rodar e estou na pista (%d, %d), minha vel: %d\n", id, cic.lin, cic.col, cic.vel);

	printf("[Thread] %d comecei\n", id);
	while (cic->vivo) {
		proxCol=cic->col;
		proxLin=cic->lin;
		pthread_mutex_lock(&m_pista);
		//quem esta na posicao que eu quero ir
		int quem=pista->p[cic->lin][(pista->d + (cic->col-1)%pista->d)%pista->d];
		pthread_mutex_unlock(&m_pista);

		/*
			Decide a proxima posicao
		*/
		if (quem == 0) {
			proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
			printf("[Thread] %d me mexi para (%d, %d)\n", id, cic->lin, proxCol);
		}
		else {
			printf("[Thread] %d esperando %d terminar sua rodada\n", id, quem);
			while(th->arrive[quem-1] == 0) continue;
			printf("[Thread] %d terminou a espera.\n", id);
			pthread_mutex_lock(&m_pista);
			int proxQuem = pista->p[cic->lin][(pista->d + (cic->col-1)%pista->d)%pista->d];
			pthread_mutex_unlock(&m_pista);
			if (proxQuem == 0) {
				proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
				printf("[Thread] %d me mexi para (%d, %d)\n", id, cic->lin, proxCol);
			}
			else {
				printf("[Thread] %d tinha gente\n", id);
			}
		}

		pthread_mutex_lock(&m_pista);
		pista->p[cic->lin][cic->col] = 0;
		pista->p[proxLin][proxCol] = id;
		pthread_mutex_unlock(&m_pista);
		cic->lin=proxLin;
		cic->col=proxCol;

		printf("[Thread] id %d foi liberado\n", id);
		th->arrive[id-1] = 1;

		pthread_mutex_lock(&m_ciclistasVivos);
		cv = ciclistasVivos;
		pthread_mutex_unlock(&m_ciclistasVivos);
		if(cv == 1) {
			printf("[Thread] %d ganhou!! Parabens!\n", id);
			break;
		}
	
		while(th->go[id-1] == 0) continue;
		th->go[id-1] = 0;
		printf("[Thread] %d acabei\n", id);
		
		if(voltaAtual>1 && voltaAnterior!=voltaAtual){
			//sorteia vel (1a volta 30km - normal)
			//nada- 30km
			//+1 - 60km
			//+2 - 90km
			float rdm = ((float)rand())/(RAND_MAX);
			//pensa nessa bagaca
			
			if(ciclistasVivos<3 && !ciclistaCorreu){
				//10% 2
				if(rdm > 0.9){
					cic->vel = 2;
					ciclistaCorreu = 1;
				}
			}
			
			else if(cic->vel == 0){
				//80% 1
				//20% 0
				if (rdm > 0.2){
					//uma casa a mais
					cic->vel = 1;
				}

			}
			else if(cic->vel == 1){
				//60% 1
				//40% 0
				if(rdm > 0.4){
					cic->vel == 1;
				}
			}
		}
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
	int ciclistasEliminados = ciclistasVivos;
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
			if(ciclistas[cic_id].vivo==0) continue;
			//printf("[Main] %d esperando arrive\n",cic_id+1);
			while (th->arrive[cic_id] == 0) continue;
			printf("[Main] %d chegou no arrive\n", cic_id+1);
		}
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->arrive[cic_id] = 0;
		}
		
		if(voltaAnterior != voltaAtual && voltaAnterior%2==0 && voltaAnterior > 1){
			//locka
			pthread_mutex_lock(&m_ciclistasVivos);
			int id = rank[ciclistasVivos-1];
			pthread_mutex_unlock(&m_ciclistasVivos);
			
			C cic = ciclistas[id-1];
			//fprintf(stderr,"lin%d  col%d  id%d\n",cic.lin,cic.col,id);
			//sou o ultimo
			printf("[Thread][morre] %d volta %d\n",id,voltaAtual);
			ciclistas[id-1].vivo = 0;	
			//exibePista(pista);
			pthread_mutex_lock(&m_pista);
			//fprintf(stderr,"pista do coiso %d\n",pista->p[cic.lin][cic.col]);
			pista->p[cic.lin][cic.col] = 0;
			//fprintf(stderr,"pista do coiso %d\n",pista->p[cic.lin][cic.col]);
			pthread_mutex_unlock(&m_pista);

			pthread_mutex_lock(&m_ciclistasVivos);
			ciclistasVivos--;
			pthread_mutex_unlock(&m_ciclistasVivos);
			//exibePista(pista);
			fprintf(stderr,"lin%d  col%d\n",cic.lin,cic.col);
		}
		
		// Aqui ninguem mexe na pista, esta todo mundo travado
		// lugar safe de colocar o print e ver certo!
		exibePista(pista);
		//muda de vorta
		voltaAnterior = voltaAtual;
		if(i%pista->d == 0) voltaAtual++;
			
		assert(ciclistasEliminados-ciclistasVivos <= 1);
		printf("[Main] %d ciclistas foram eliminados\n", ciclistasEliminados-ciclistasVivos);
		ciclistasEliminados = ciclistasVivos;
		printf("*****************************\n");

		// Aqui é permito eles rodarem de novo
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->go[cic_id] = 1;
		}
		
	}
	printf("[Main] FIM DO WHILE vivos-> %d\n",ciclistasVivos);

	for (int i = 0; i < P->n; i++) {
		pthread_join(th->tids[i], NULL);
	}
	exibePista(pista);
	printf("[Main] Fim da corrida\n");
}

void montaPista(int d, int n, Pista* P)
{
	int i;
	ciclistas = (C*) malloc(P->n * sizeof(C));
	rank = (int*) malloc(n*sizeof(int));

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
		P->p[i%5][i/5 + 1] = v[i];
		ciclistas[i].pos = i;
		rank[i] = v[i];
		// Olho nele!
		ciclistas[v[i]-1].lin = i%5;
		ciclistas[v[i]-1].col = i/5 + 1;
		ciclistas[v[i]-1].vel = 0; // 30km/h em milissegundos.
	}
} 


int main(int argc, char** argv)
{
	srand(time(0));
	ciclistaCorreu = 0;
	pista = (Pista *)malloc(sizeof(Pista));
	pista->d = atoi(argv[1]); //tem q ver se eh ```d n``` ou ```n d```
	pista->n = atoi(argv[2]); 
	pista->n_vol = 0;
	pthread_mutex_init(&m_pista, NULL);
	pthread_mutex_init(&m_ciclistasVivos, NULL);
	montaPista(pista->d,pista->n,pista);
	//exibePista(pista);
	start_run(pista);
	pthread_mutex_destroy(&m_pista);
	pthread_mutex_destroy(&m_ciclistasVivos);
}
