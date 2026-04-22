#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define TAG_RESULTADO 1

/* A mesma função otimizada que já estávamos usando */
int primo(long int n) {
    if (n <= 1) return 0;
    
    int limite = (int)sqrt(n);
    for (int i = 3; i <= limite; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int meu_ranque, num_procs;
    long int n;
    int cont = 0, total = 0;
    double t_inicial, t_final;
    MPI_Status estado;

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
     * DIVISÃO NAIVE (CÍCLICA DA AULA)
     * ========================================================== */
    long int inicio = 3 + (meu_ranque * 2);
    long int salto = num_procs * 2;

    for (long int i = inicio; i <= n; i += salto) {
        if (primo(i) == 1) {
            cont++;
        }
    }

    /* ==========================================================
     * COMUNICAÇÃO PONTO A PONTO (Exigência do Trabalho)
     * Substituindo o MPI_Reduce da aula
     * ========================================================== */
    if (meu_ranque == 0) {
        total = cont; /* Mestre adiciona sua própria contagem */
        int cont_recebido;
        
        /* Mestre recebe a contagem de cada escravo individualmente */
        for (int origem = 1; origem < num_procs; origem++) {
            MPI_Recv(&cont_recebido, 1, MPI_INT, origem, TAG_RESULTADO, MPI_COMM_WORLD, &estado);
            total += cont_recebido;
        }
    } else {
        /* Escravos enviam sua contagem para o Mestre usando MPI_Send */
        MPI_Send(&cont, 1, MPI_INT, 0, TAG_RESULTADO, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        if (n >= 2) total += 1; /* Acrescenta o dois, que é primo par */
        
        printf("=========================================\n");
        printf("[NAIVE - CICLICO] Quant. de primos entre 1 e %ld: %d\n", n, total);
        printf("Tempo de execucao: %1.3f segundos\n", t_final - t_inicial);      
        printf("=========================================\n");
    }

    MPI_Finalize();
    return 0;
}
