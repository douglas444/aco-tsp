typedef struct parametros
{
    double alpha, beta, taxaEvapo, feroInicial;
    int iteracoes, populacao;
}Parametros;

void executaAlgoritmo(Parametros p, int qtdCidades, double **distancias,
        int *rotaGlobal, double *custoRotaGlobal);
