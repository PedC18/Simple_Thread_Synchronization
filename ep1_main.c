/*
    O programa ep1_main.c foi compilado nas maquinas linux dos labs de graduação sem erros com o seguinte comando:
        $gcc ep1_main.c spend_time.c spend_time.h -o ep1_main -lpthread -lm
    E testado no formato:
        $./ep1_main < teste.txt
    
    Aluno: Pedro Cimini Mattos de Freitas - 2019007031
*/

#include "spend_time.h"
#include "stdlib.h"
#include "stdio.h"
#include "pthread.h"
#include "string.h"

pthread_mutex_t Mutex;
pthread_cond_t Available;

//Vetor binário: 0 o recurso está disponível, 1 o recurso está sendo utilizado.
int Recursos[8];

typedef struct { //GUARDA TODAS AS INFORMAÇÕES NECESSÁRIAS PARA EXECUTAR AS THREADS
    int tcritico;
    int tlivre;
    int pid;

    /*Guardará os indices dos recursos pedidos especificados pela entrada*/
    int idx_recursos[8];
    /*Guarda o numero de "idx"s passados*/
    int tamanho;

} ThreadArgs;

/*Funcao que inicializa todas as posições do vetor global Recursos*/
void init_recursos(){
    memset(Recursos,0,8);
}

/*Funcao auxiliar criada para checar se todos os recursos pedidos por uma determinada thread
  estão disponiveis. Se sim, retorna 1. Se algum deles não esta, retorna 0*/
int check_recursos(ThreadArgs *arg){
    int available = 1;
    for(int i = 0; i<arg->tamanho; i++){
        if(Recursos[arg->idx_recursos[i]] == 1){
            available = 0;
            break;
        }
    }

    return available;
}


void trava_recursos(ThreadArgs *arg){
    pthread_mutex_lock(&Mutex);

    /*Caso os recursos nao estão disponiveis, a condição é disparada e a thread aguarda
      a liberação de recursos de outra thread, o que só acontece na função libera_recuros()*/
    while(check_recursos(arg) == 0){
        pthread_cond_wait(&Available, &Mutex);
    }

    /*Atualiza o vetor Recursos, "settando" os recursos travados*/
    for(int i = 0; i < arg->tamanho ; i++){
            Recursos[arg->idx_recursos[i]] = 1;
    } 

    pthread_mutex_unlock(&Mutex);
};

/*Eu não vi outra forma a não ser passar ThreadArgs como parâmetro, pois não encontrei uma forma
  de liberar APENAS os recursos utilizados na thread de outra forma. Entendo que a ideia é que nenhum
  parametro fosse passado para essa função, porém não consegui fazer funcionar de forma diferente*/

void libera_recursos(ThreadArgs *arg){
    pthread_mutex_lock(&Mutex);

    /*Libera os recursos utilizados apenas por essa thread*/
    for(int i = 0; i< arg->tamanho ; i++){
            Recursos[arg->idx_recursos[i]] = 0;
    }

    /*Sinaliza a todos que estão esperando que recursos foram liberados*/
    pthread_cond_broadcast(&Available);

    pthread_mutex_unlock(&Mutex);
};

/*Rotina especificada no enunciado do exercicio. Recebe como parametro todos os dados
  necessários da thread por meio do struct ThreadArgs*/
void* function(void *arg){
    ThreadArgs *Arg = (ThreadArgs*)arg;

    spend_time(Arg->pid,NULL,Arg->tlivre);
    trava_recursos(Arg);
    
    spend_time(Arg->pid,"C",Arg->tcritico);
    libera_recursos(Arg);     

    pthread_exit(EXIT_SUCCESS);       

};

int main(){
   
    pthread_mutex_init(&Mutex,NULL);
    pthread_cond_init(&Available,NULL);
    
    init_recursos();
    
    pthread_t thread;

    pthread_t vec_threads[4]; //Utilizado para dar join nas threads e previnir o encerramento precoce do programa
    int thread_count = 0;

    while(1){
        /*Inicializa um ponteiro de ThreadArgs*/
        ThreadArgs *Arg = malloc(sizeof(ThreadArgs));
        Arg->tamanho = 0;
    	memset(Arg->idx_recursos, 0, 8);
    	
	    char input[50] = {0};
        
        //Necessário para a separação de recursos
    	char *recursos = {0};
      	
        if(fgets(input,50,stdin) == NULL){ 
            break;
        };

        /*Quebra o input em partes que são guardadas nas variáveis corretas*/
        Arg->pid = atoi(strtok(input," "));
        Arg->tlivre = atoi(strtok(NULL," "));
        Arg->tcritico = atoi(strtok(NULL," "));
        recursos = strtok(NULL,"\n");
        
        /*O codigo abaixo recebe uma string de recursos, quebra ela de forma que cada recurso seja separado
          e os insere em ordem de chegada no vetor Arg->idx_recursos*/
        int i = 0;
        
        char *aux = {0};
        aux = strtok(recursos, " ");
        while(aux != NULL){
            Arg->idx_recursos[i] = atoi(aux);
            aux = strtok(NULL, " ");
            Arg->tamanho++;
            i++;
        }
        
        pthread_create(&thread,  NULL , function, Arg);
        
        /*Guarda e conta as threads*/
        vec_threads[thread_count] = thread;
        thread_count++;
    }
    
    //Realiza o join em todas as threads criadas
    for(int i = 0; i<4 ; i++){    
        pthread_join(vec_threads[i], NULL);
    }
    
    return 0;
}
