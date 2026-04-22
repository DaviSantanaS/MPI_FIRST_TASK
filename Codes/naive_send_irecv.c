#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

#define TAG_RESULTADO 1

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
    MPI_Request req_recv; /* Requisição para o Irecv */

    if (argc < 2) {
        return 0;
    } else {
        n = strtol(argv[1], (char **) NULL, 10);
    }

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &meu_ranque);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    t_inicial = MPI_Wtime();

    long int inicio = 3 + (meu_ranque * 2);
    long int salto = num_procs * 2;

    for (long int i = inicio; i <= n; i += salto) {
        if (primo(i) == 1) {
            cont++;
        }
    }

    /* === COMUNICAÇÃO: SEND + IRECV === */
    if (meu_ranque == 0) {
        total = cont; 
        int cont_recebido;
        for (int k = 1; k < num_procs; k++) {
            /* SUBSTITUÍDO: Mestre recebe com Recebimento Não-Bloqueante */
            MPI_Irecv(&cont_recebido, 1, MPI_INT, MPI_ANY_SOURCE, TAG_RESULTADO, MPI_COMM_WORLD, &req_recv);
            MPI_Wait(&req_recv, &estado); /* Espera a mensagem chegar para somar */
            total += cont_recebido;
        }
    } else {
        MPI_Send(&cont, 1, MPI_INT, 0, TAG_RESULTADO, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (meu_ranque == 0) {
        t_final = MPI_Wtime();
        if (n >= 2) total += 1; 
        
        printf("=========================================\n");
        printf("[NAIVE - CICLICO] Quant. de primos entre 1 e %ld: %d\n", n, total);
        printf("Tempo de execucao: %1.3f segundos\n", t_final - t_inicial);      
        printf("=========================================\n");
    }

    MPI_Finalize();
    return 0;
}
