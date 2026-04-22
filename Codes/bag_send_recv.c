/*bag_send_recv.c ele ja tem o bag send recv*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define TAMANHO 500000
#define TAG_TRABALHO 1
#define TAG_FIM 99

/* Função otimizada: calcula o limite apenas uma vez */
int primo(int n) {
    if (n <= 1) return 0;
    
    int limite = (int)sqrt(n);
    for (int i = 3; i <= limite; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int meu_ranque, num_procs;
    int n, inicio, dest;
    int cont = 0, total = 0;
    int raiz = 0;
    double t_inicial, t_final;
    MPI_Status estado;

    /* Verifica os argumentos */
    if (argc < 2) {
        printf("Uso: mpirun -np <processos> %s <limite_superior>\n", argv[0]);
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (num_procs < 2) {
        if (meu_ranque == 0) {
            printf("Este programa deve ser executado com no minimo dois processos.\n");
        }
        MPI_Finalize();
        return 1;
    }

    t_inicial = MPI_Wtime();

    /* ==========================================================
     * CÓDIGO DO MESTRE (Rank 0)
     * ========================================================== */
    if (meu_ranque == 0) { 
        int trabalhadores_ativos = 0;
        inicio = 3; // Começa a verificar a partir do 3 (ímpares)

        /* 1. Distribui a primeira leva de trabalho para os escravos */
        for (dest = 1; dest < num_procs; dest++) {
            if (inicio <= n) {
                MPI_Send(&inicio, 1, MPI_INT, dest, TAG_TRABALHO, MPI_COMM_WORLD);
                trabalhadores_ativos++;
                inicio += TAMANHO;
            } else {
                MPI_Send(&inicio, 1, MPI_INT, dest, TAG_FIM, MPI_COMM_WORLD);
            }
        }

        /* 2. Escuta os resultados e envia novos pacotes sob demanda */
        while (trabalhadores_ativos > 0) {
            MPI_Recv(&cont, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
            total += cont;
            dest = estado.MPI_SOURCE; 
            
            if (inicio <= n) {
                MPI_Send(&inicio, 1, MPI_INT, dest, TAG_TRABALHO, MPI_COMM_WORLD);
                inicio += TAMANHO;
            } else {
                /* Não há mais trabalho, dispensa o trabalhador */
                MPI_Send(&inicio, 1, MPI_INT, dest, TAG_FIM, MPI_COMM_WORLD);
                trabalhadores_ativos--;
            }
        }
    } 
    /* ==========================================================
     * CÓDIGO DO ESCRAVO (Ranks > 0)
     * ========================================================== */
    else { 
        while (1) {
            MPI_Recv(&inicio, 1, MPI_INT, raiz, MPI_ANY_TAG, MPI_COMM_WORLD, &estado);
            
            if (estado.MPI_TAG == TAG_FIM) {
                break; // Mestre avisou que o trabalho acabou
            }

            cont = 0;
            // Verifica os ímpares dentro do bloco [inicio, inicio + TAMANHO[
            for (int i = inicio; i < (inicio + TAMANHO) && i <= n; i += 2) {
                if (primo(i) == 1) {
                    cont++;
                }
            }
            
            /* Envia os primos encontrados neste bloco de volta para o Mestre */
            MPI_Send(&cont, 1, MPI_INT, raiz, TAG_TRABALHO, MPI_COMM_WORLD);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    
    /* Mestre finaliza a contagem e imprime os resultados */
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        if (n >= 2) total += 1; /* Acrescenta o número 2, que é o único primo par */
        
        printf("=========================================\n");
        printf("Quant. de primos entre 1 e %d: %d\n", n, total);
        printf("Tempo de execucao: %1.3f segundos\n", t_final - t_inicial);      
        printf("=========================================\n");
    }

    MPI_Finalize();
    return 0;
}
