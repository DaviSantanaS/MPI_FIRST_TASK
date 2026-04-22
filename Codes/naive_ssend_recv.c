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

    if (meu_ranque == 0) {
        total = cont; 
        int cont_recebido;
        for (int origem = 1; origem < num_procs; origem++) {
            MPI_Recv(&cont_recebido, 1, MPI_INT, origem, TAG_RESULTADO, MPI_COMM_WORLD, &estado);
            total += cont_recebido;
        }
    } else {
        /* SUBSTITUÍDO PARA SSEND */
        MPI_Ssend(&cont, 1, MPI_INT, 0, TAG_RESULTADO, MPI_COMM_WORLD);
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
