#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

typedef struct C //ciclista
{
	int vel; // velocidade -> qauntos ms ele vai demorar p 1 metro nessa volta
	int lin; //linha da pista
	int col; // coluna da pista
	int vivo;
	int pos; //posicao
	int quads; //volta
	int tempoLargada;
	int quadVelAtualizada;
	int quebrou;
	int voltaQuebrou;
} C;

typedef struct {
	int d; //largura da pista
	int n; //quantidade de ciclistas
	int n_vol; 
	int** p; // <- do professor
} Pista;

//[ [2, 1, 4, 7, 6], [5, 3], [], [] ]

typedef struct {
	pthread_t* tids;
	int* ids;
	int* arrive;
	int* go;
} ThreadHelp;

int *rank;
int cpo; //ciclistas que passaram a origem
int ciclistasVivos;
int nRank;
int voltaAtual = 0;
int voltaAnterior = -1;
int tempo = 0;
C* ciclistas;
Pista* pista;
ThreadHelp* th;
pthread_mutex_t m_pista;
pthread_mutex_t m_ciclistaCorreu;
int ciclistaCorreu;
int velocidadeDeAtualizacao = 1; // 1 -> 60ms , 2-> 20ms

void exibePista(Pista* P) {
	//fprintf(stderr,"volta %d\n",voltaAtual);

	pthread_mutex_lock(&m_pista);
	for(int j=0;j<10;j++){
		for(int k=0;k<(P->d);k++){
			if (P->p[j][k] == 0) {
	//			fprintf(stderr,"|  ");
			}
			else {
				//fprintf(stderr,"|%2d",P->p[j][k]);
			}
		}
		//fprintf(stderr,"|\n");
	}
	pthread_mutex_unlock(&m_pista);
}

void exiberank() {
	for (int i = 0; i < pista->n; i++) {
		C cic = ciclistas[rank[i] - 1];
		//fprintf(stderr,"|%2d(%2d)",rank[i], cic.quads);
	}
	//fprintf(stderr,"|\n");
}

void anda60ms(int rodada, C* cic, int id) {
	int quemFrente;
	int quemFrente2;
	int proxLin = cic->lin;
	int proxCol = cic->col;
	float rdm;
	int andei = 0;
	// Define as novas posicoes
	if (rodada % cic->vel == 0) {
		pthread_mutex_lock(&m_pista);
		quemFrente=pista->p[cic->lin][(pista->d + (cic->col-1)%pista->d)%pista->d];
		pthread_mutex_unlock(&m_pista);

		/*
			Decide a proxima posicao
		*/
		if(quemFrente > 0){
			while(th->arrive[quemFrente-1] == 0) continue;
		}
		
		pthread_mutex_lock(&m_pista);
		quemFrente=pista->p[cic->lin][(pista->d + (cic->col-1)%pista->d)%pista->d];
		pthread_mutex_unlock(&m_pista);
		if(cic->vel==1){ // 60 por hora
			//ve os 2
			if(quemFrente == 0){
				//pode andar
				proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
				andei = 1;
			}
			//diminui vel
			else {
				//fprintf(stderr,"[Thread] %d nao andei a 60\n",id);
				//atualiza vel

				//ULTRAPASSAGEM
				if (cic->lin + 1 < pista->d) {
					pthread_mutex_lock(&m_pista);
					quemFrente=pista->p[cic->lin + 1][(pista->d + (cic->col)%pista->d)%pista->d];
					quemFrente2=pista->p[cic->lin + 1][(pista->d + (cic->col-1)%pista->d)%pista->d];
					pthread_mutex_unlock(&m_pista);

					if(quemFrente > 0){
						while(th->arrive[quemFrente-1] == 0) continue;
					}
					if(quemFrente2 > 0){
						while(th->arrive[quemFrente2-1] == 0) continue;
					}
					
					//ve os 2
					if(quemFrente == 0 && quemFrente2 == 0){
						//pode andar
						//fprintf(stderr,"[Thread] %d ultrapassei!\n",id);
						proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
						proxLin = cic->lin + 1;
						andei = 1;
					}
					//diminui vel
					else {
						//fprintf(stderr,"[Thread] %d nao andei a 60\n",id);
						//atualiza vel
						cic->vel = 2;
						return;
					}

				} else {
					cic->vel = 2;
					return;
				}
			}
		}
		// Aqui eu andei!
		if(cic->vel == 2){ // velocidade 30km/h
			if (quemFrente == 0) {
				proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
				andei = 1;
			}
			else {
				//fprintf(stderr,"[Thread] %d nao andei a 30\n",id);
			}
		}
		cic->quads++;
	}

	pthread_mutex_lock(&m_pista);
	pista->p[cic->lin][cic->col] = 0;
	pista->p[proxLin][proxCol] = id;
	cic->lin=proxLin;
	cic->col=proxCol;

	if ((cic->quads/pista->d) % 6 == 0 && ciclistasVivos > 5 && cic->quads > pista->d) {
		rdm = ((float)rand())/(RAND_MAX);

		if (rdm < 0.05) {
			//fprintf(stderr, "[Thread][quebrou] %d (%f)\n",id, rdm);
			cic->quebrou = 1;
			cic->voltaQuebrou = cic->quads/pista->d;
			cic->vivo = 0;
			pista->p[cic->lin][cic->col] = 0;
		}
	}
	pthread_mutex_unlock(&m_pista);
}

void atualizaVel60ms(C* cic, int id) {
	float rdm;
	// Atualiza a velocidade
	if(cic->quads >= pista->d && cic->quads % pista->d == 0 && cic->quads != cic->quadVelAtualizada){
		cic->quadVelAtualizada = cic->quads;
		//sorteia vel (1a volta 30km - normal)
		//+1- 30km
		//+2 - 60km
		//+3 - 90km
		rdm = ((float)rand())/(RAND_MAX);
	//	printf("[Thread] %d vai mudar de vel. prod: %f\n", id, rdm);
		//pensa nessa bagaca
		
		if(ciclistasVivos<3){
			//10% 3
			pthread_mutex_lock(&m_ciclistaCorreu);
			if (!ciclistaCorreu) {
				if(rdm > 0.9){
					cic->vel = 3;
					ciclistaCorreu = 1;
				}
			}
			else if(cic->vel == 1) { // estamos em 60 por hora
				//80% 2
				//20% 1
				if(rdm > 0.6){
					//uma casa a mais
					cic->vel = 2;
				}

			}
			else if(cic->vel == 2) { // estamos a trinta por hora
				//60% 2
				//40% 1
				if (rdm > 0.2){
					cic->vel = 1;
				}
			}
			pthread_mutex_unlock(&m_ciclistaCorreu);
		}
		else if(cic->vel == 2) { // estamos em trinta por hora
			//80% 2
			//20% 1
			if (rdm > 0.2){
				//uma casa a mais
				cic->vel = 1; // vamos para 60 por hora
			}
		}
		else if(cic->vel == 1) { // estamos a 60 por hora
			//60% 2
			//40% 1
			if(rdm > 0.6){
				cic->vel = 2; // vamos para 30 por hora.
			}
		}
	}	
}

void anda20ms(int rodada, C* cic, int id) {
	int quemFrente;
	int quemFrente2;
	int proxLin = cic->lin;
	int proxCol = cic->col;
	float rdm;

	// Define as novas posicoes
	if (rodada % cic->vel == 0) {
		pthread_mutex_lock(&m_pista);
		quemFrente=pista->p[cic->lin][(pista->d + (cic->col-1)%pista->d)%pista->d];
		pthread_mutex_unlock(&m_pista);

		/*
			Decide a proxima posicao
		*/
		if(quemFrente > 0){
			while(th->arrive[quemFrente-1] == 0) continue;
		}
		
		pthread_mutex_lock(&m_pista);
		quemFrente=pista->p[cic->lin][(pista->d + (cic->col-1)%pista->d)%pista->d];
		pthread_mutex_unlock(&m_pista);
		if(cic->vel == 3){ // 60 por hora
			//ve os 2
			if(quemFrente == 0){
				//pode andar
				proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
				cic->quads++;
			}
			//diminui vel
			else {
				//fprintf(stderr,"[Thread] %d nao andei a 60\n",id);
				//atualiza vel

				//ULTRAPASSAGEM
				if (cic->lin + 1 < pista->d) {
					pthread_mutex_lock(&m_pista);
					quemFrente=pista->p[cic->lin + 1][(pista->d + (cic->col)%pista->d)%pista->d];
					quemFrente2=pista->p[cic->lin + 1][(pista->d + (cic->col-1)%pista->d)%pista->d];
					pthread_mutex_unlock(&m_pista);

					if(quemFrente > 0){
						while(th->arrive[quemFrente-1] == 0) continue;
					}
					if(quemFrente2 > 0){
						while(th->arrive[quemFrente2-1] == 0) continue;
					}
					
					//ve os 2
					if(quemFrente == 0 && quemFrente2 == 0){
						//pode andar
						//fprintf(stderr,"[Thread] %d ultrapassei!\n",id);
						proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
						proxLin = cic->lin + 1;
						cic->quads++;
					}
					//diminui vel
					else {
						//fprintf(stderr,"[Thread] %d nao andei a 60\n",id);
						//atualiza vel
						cic->vel = 6;
						return;
					}

				} else {
					cic->vel = 6;
					return;
				}
			}
		}
		// Aqui eu andei!
		if(cic->vel == 6){ // velocidade 30km/h
			if (quemFrente == 0) {
				proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
				cic->quads++;
			}
			else {
				//fprintf(stderr,"[Thread] %d nao andei a 30\n",id);
			}
		}
		if(cic->vel == 2) { // velocidade 90 km/h
			if (quemFrente == 0) {
				proxCol = (pista->d + (cic->col-1)%pista->d)%pista->d;
				cic->quads++;
			}
			else {
				//fprintf(stderr,"[Thread] %d nao andei a 90\n",id);
			}
		}
	}

	pthread_mutex_lock(&m_pista);
	pista->p[cic->lin][cic->col] = 0;
	pista->p[proxLin][proxCol] = id;
	cic->lin=proxLin;
	cic->col=proxCol;

	if ((cic->quads/pista->d) % 6 == 0 && ciclistasVivos > 5 && cic->quads > pista->d) {
		rdm = ((float)rand())/(RAND_MAX);

		if (rdm < 0.05) {
		//	fprintf(stderr, "[Thread][quebrou] %d (%f)\n",id, rdm);
			cic->quebrou = 1;
			cic->voltaQuebrou = cic->quads/pista->d;
			cic->vivo = 0;
			pista->p[cic->lin][cic->col] = 0;
		}
	}
	pthread_mutex_unlock(&m_pista);
}

void atualizaVel20ms(C* cic, int id) {
	float rdm;
	// Atualiza a velocidade
	//fprintf(stderr, "[Thread] %d vel atual %d\n", id, cic->vel);
	if(cic->quads >= pista->d && cic->quads % pista->d == 0 && cic->quads != cic->quadVelAtualizada){
		cic->quadVelAtualizada = cic->quads;
		//sorteia vel (1a volta 30km - normal)
		//+1- 30km
		//+2 - 60km
		//+3 - 90km
		rdm = ((float)rand())/(RAND_MAX);
		//printf("[Thread] %d vai mudar de vel. prod: %f\n", id, rdm);
		//pensa nessa bagaca
		

		if (cic->vel == 2) { // estamos a 90km/h, continue até o fim!
			return;
		}
		else if(cic->vel == 6) { // estamos em trinta por hora
			//80% 2
			//20% 1
			if (rdm > 0.2){
				//uma casa a mais
				cic->vel = 3; // vamos para 60 por hora
			}
		}
		else if(cic->vel == 3) { // estamos a 60 por hora
			//60% 2
			//40% 1
			if(rdm > 0.6){
				cic->vel = 6; // vamos para 30 por hora.
			}
		}
	}
}

// vetor de ciclistas 
void* ciclista_thread(void* i) {
	int id = *((int *) i);
	C* cic = &ciclistas[id-1];
	int rodada = 0;
	//printf("[Thread] Sou o ciclista %d e comecei a rodar e estou na pista (%d, %d), minha vel: %d\n", id, cic.lin, cic.col, cic.vel);

//	printf("[Thread] %d comecei\n", id);

	while (cic->vivo) {
		rodada++;
 
		if (velocidadeDeAtualizacao == 1) {
			anda60ms(rodada, cic, id);
			atualizaVel60ms(cic, id);
		}
		else {
			anda20ms(rodada, cic, id);
			atualizaVel20ms(cic, id);
		}

		if(cic->quads >= pista->d && cic->quads % pista->d == 0 && cic->quads != cic->quadVelAtualizada){
			cic->tempoLargada = tempo;
		}
		
		if(ciclistasVivos == 1) {
			//printf("[Thread] %d ganhou!! Parabens!\n", id);
			break;
		}

		th->arrive[id-1] = 1;

		while(th->go[id-1] == 0) continue;
		th->go[id-1] = 0;
	}

	return NULL;
}

// runner -> loop
void start_run(Pista* P){
	exibePista(pista);
	th = (ThreadHelp*) malloc(sizeof(ThreadHelp));
	th->tids = (pthread_t*) malloc(pista->n * sizeof(pthread_t));
	th->ids = (int*) malloc(pista->n * sizeof(int));
	th->arrive = (int*) malloc(pista->n * sizeof(int));
	th->go = (int*) malloc(pista->n * sizeof(int));

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
		// Aqui eu espero todos os ciclistas rodarem
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			if(ciclistas[cic_id].vivo==0) continue; // não precisa esperar um ciclista morto
			//printf("[Main] %d esperando arrive\n",cic_id+1);
			while (th->arrive[cic_id] == 0) continue;
			//printf("[Main] %d chegou no arrive\n", cic_id+1);
		}
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->arrive[cic_id] = 0;
		}

		// atualiza o rank
		for (int i = nRank; i >= 0; i--) {
			int id = rank[i];
			//printf("id: %d \n", id);
			C* cic = &ciclistas[ id - 1 ];
			int pos_rank = cic->pos;
			for (int j = i; j>=0; j--) {
				int next_cic_id = rank[j];
				//printf("id j: %d \n", next_cic_id);
				C* next_cic = &ciclistas[next_cic_id - 1];
				int temp;
				if (cic->quads > next_cic->quads) {
					temp = rank[j];
					// atualizamos a nossa posição no rank.
					rank[j] = id;
					rank[pos_rank] = temp;

					// atualizas a posica
					ciclistas[next_cic_id - 1].pos = pos_rank;
					cic->pos = j;
					pos_rank = cic->pos;
					// fprintf(stderr,"A: ");
					// exiberank();
				}
				else if (!next_cic->vivo) {
					int ondeEuMorri = next_cic->pos;
					int quemEuMorri = rank[ondeEuMorri];

					for (int k = ondeEuMorri+1; k <= nRank; k++) {
						int t = rank[k];
						C* _cic = &ciclistas[t - 1];
						_cic->pos = k-1;
						rank[k-1] = t;
					}

					rank[nRank] = quemEuMorri;
					next_cic->pos = nRank;
					nRank--;
					ciclistasVivos--;
					i--;
					// fprintf(stderr,"B: ");
					// exiberank();
				}
			}
		}
		exiberank();

		//i++;
		if (velocidadeDeAtualizacao == 1) tempo += 60;
		else tempo += 20;

		//Eliminamos o ultimos
		if(voltaAnterior != voltaAtual && voltaAnterior%2==0 && voltaAnterior > 1){
			//locka

			for (int i = nRank; i >= 0; i--) {
				int id = rank[i];
				
				C cic = ciclistas[id-1];

				if (!cic.vivo) {

					nRank--;
					continue;
				}
				//fprintf(stderr,"lin%d  col%d  id%d\n",cic.lin,cic.col,id);
				//sou o ultimo
				//printf("[Thread][morre] %d volta %d\n", id, voltaAtual);
				ciclistas[id-1].vivo = 0;	
				//exibePista(pista);
				pthread_mutex_lock(&m_pista);
				//fprintf(stderr,"pista do coiso %d\n",pista->p[cic.lin][cic.col]);
				pista->p[cic.lin][cic.col] = 0;
				//fprintf(stderr,"pista do coiso %d\n",pista->p[cic.lin][cic.col]);
				pthread_mutex_unlock(&m_pista);

				nRank--;
				ciclistasVivos--;
				//exibePista(pista);
				break;
			}
		}
		
		// Aqui ninguem mexe na pista, esta todo mundo travado
		// lugar safe de colocar o print e ver certo!
		exibePista(pista);

		//muda de vorta
		voltaAnterior = voltaAtual;
		voltaAtual = ciclistas[ rank[nRank]-1 ].quads / pista->d;
		//if(i%pista->d == 0) ++;

		//printf("[Main] %d ciclistas foram eliminados\n", ciclistasEliminados-ciclistasVivos);
		ciclistasEliminados = ciclistasVivos;
		//printf("*****************************\n");


		// Atualizamos a velocidade de atualização de exibição da pista
		if (ciclistaCorreu == 1 && velocidadeDeAtualizacao == 1) {
			//fprintf(stderr,"-----------------------------\n");
			//fprintf(stderr,"----------- att vel ---------\n");
			//fprintf(stderr,"-----------------------------\n");
			// muda a velodade de atualizacao
			velocidadeDeAtualizacao = 2;

			for (int i = 0; i < pista->n; i++) {
				C* cic = &ciclistas[i];

				if (cic->vel == 1) { // 60km/h
					cic->vel = 3; // a cada 60 ms anda 1 metro
				}
				else if (cic->vel == 2) { // 30km/h
					cic->vel = 6; // a cada 120ms anda 1 metro
				}
				else if (cic->vel == 3) { // 90km/h
					cic->vel = 2; // a cada 40ms anda 1 metro
				}
			}
		}

		//sleep(1);
		// Aqui é permito eles rodarem de novo
		for (int cic_id = 0; cic_id < pista->n; cic_id++) {
			th->go[cic_id] = 1;
		}
		
	}
	//printf("[Main] FIM DO WHILE vivos-> %d\n",ciclistasVivos);

	for (int i = 0; i < P->n; i++) {
		pthread_join(th->tids[i], NULL);
	}
	exibePista(pista);
	//printf("[Main] Fim da corrida\n");
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
	nRank = n-1;
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
		ciclistas[i].quads = 0;

		P->p[i%5][i/5 + 1] = v[i];

		ciclistas[ v[i]-1 ].pos = i;
		rank[i] = v[i];
		// Olho nele!
		ciclistas[v[i]-1].lin = i%5;
		ciclistas[v[i]-1].col = i/5 + 1;
		ciclistas[v[i]-1].vel = 2; // 30km/h
		ciclistas[v[i]-1].quadVelAtualizada = -1; // 30km/h
		ciclistas[v[i]-1].quebrou = 0;
		ciclistas[v[i]-1].voltaQuebrou = 0;
		ciclistas[v[i]-1].tempoLargada = 0;
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
	pthread_mutex_init(&m_ciclistaCorreu, NULL);
	montaPista(pista->d,pista->n,pista);
	start_run(pista);
	pthread_mutex_destroy(&m_ciclistaCorreu);
	pthread_mutex_destroy(&m_pista);
	// acabou!

	int i;
	free(th->tids);
	free(th->ids);
	free(th->arrive);
	free(th->go);
	free(th);
	free(ciclistas);
	free(rank);
	for (i=0; i<10; i++){
		free(pista->p[i]);
	}
	free(pista->p);
	free(pista);
}
