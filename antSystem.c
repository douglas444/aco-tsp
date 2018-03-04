#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "antSystem.h"

typedef struct candidato
{
    int indice;
    double probabilidade;
}Candidato;

void atualizaFeromonio(Parametros p, double **feromonio, double custo, int *rota, int qtdCidades);
void constroiRota(Parametros p, int qtdCidades, double **distancias,
        double **feromonio, int cidadeAtual, double *custo, int *rota);
void calculaProbabilidades(Candidato *candidatas, int cidadeAtual,
        Parametros p, double **feromonio, double **distancias, int qtdCandidatas);
int sorteiaProximoVertice(Candidato *candidatas, int qtdCandidatas);
void quickSort(Candidato *candidatas, int n);
void ordena(Candidato *candidatas, int esq, int dir);
void particiona(Candidato *candidatas, int esq, int dir, int *i, int *j);

void executaAlgoritmo(Parametros p, int qtdCidades, double **distancias,
        int *rotaGlobal, double *custoRotaGlobal)
{
    int i, j, k, *rotaFormiga, *rotaIteracao;
    double **feromonio, custoRotaIteracao, custoRotaFormiga;

    srand((unsigned)time(NULL));

    rotaFormiga = (int*) malloc(sizeof(int) * qtdCidades);
    rotaIteracao = (int*) malloc(sizeof(int) * qtdCidades);
    feromonio = (double**) malloc(sizeof(double*) * qtdCidades);

    if(rotaFormiga == NULL || rotaIteracao == NULL || feromonio == NULL)
    {
        exit(1);
    }

    //inicialização da matriz de feromônio
    for(i = 0; i < qtdCidades; ++i)
    {
        feromonio[i] = (double*) malloc(sizeof(double) * qtdCidades);
        if(feromonio[i] == NULL)
        {
            exit(1);
        }
        for(j = 0; j < qtdCidades; ++j)
        {
            if(i != j)
                feromonio[i][j] = p.feroInicial;
            else
                feromonio[i][j] = 0;
        }
    }

    *custoRotaGlobal = 0;

    //laço principal
    for(i = 0; i < p.iteracoes; ++i)
    {
        custoRotaIteracao = 0;

        //laço da população
        for(j = 0; j < p.populacao; ++j)
        {
            custoRotaFormiga = 0;

            //a formiga 'j' contrói sua rota
            constroiRota(p, qtdCidades, distancias, feromonio, j%qtdCidades,
                    &custoRotaFormiga, rotaFormiga);

            //a rota da formiga 'j' é armazenada se for a melhor até então
            if(custoRotaFormiga < custoRotaIteracao || custoRotaIteracao == 0)
            {
                custoRotaIteracao = custoRotaFormiga;
                for(k = 0; k < qtdCidades; ++k)
                {
                    rotaIteracao[k] = rotaFormiga[k];
                }
            }
        }

        //faz as atualizações do feromônio
        atualizaFeromonio(p, feromonio, custoRotaIteracao, rotaIteracao, qtdCidades);

        //melhor rota da iteração passa a ser a melhor rota global se assim for
        if(custoRotaIteracao < *custoRotaGlobal || *custoRotaGlobal == 0)
        {
            *custoRotaGlobal = custoRotaIteracao;
            for(j = 0; j < qtdCidades; ++j)
            {
                rotaGlobal[j] = rotaIteracao[j];
            }
        }
    }

    free(rotaFormiga);
    free(rotaIteracao);
    for(i = 0; i < qtdCidades; ++i)
    {
        free(feromonio[i]);
    }

}

void atualizaFeromonio(Parametros p, double **feromonio, double custo, int *rota, int qtdCidades)
{
    int i, j;

    //Evapora
    for(i = 0; i < qtdCidades; ++i)
    {
        for(j = 0; j < qtdCidades; ++j){
            if(i != j)
                feromonio[i][j] *= 1 - p.taxaEvapo;
        }
    }

    //Deposita
    for(i = 0; i < qtdCidades - 1; ++i)
    {
        feromonio[rota[i]][rota[i+1]] += 1/custo;
        feromonio[rota[i+1]][rota[i]] += 1/custo;
    }
}

void constroiRota(Parametros p, int qtdCidades, double **distancias,
        double **feromonio, int cidadeAtual, double *custo, int *rota)
{
    int *foiVisitada, i, j, qtdCandidatas;
    Candidato *candidatas = NULL;

    foiVisitada = (int*) calloc(qtdCidades, sizeof(int));
    foiVisitada[cidadeAtual] = 1;

    //armazena cidade de partida
    rota[0] = cidadeAtual;

    //laço para preencher demais posições do vetor rota
    for(i = 1; i < qtdCidades; ++i)
    {
        qtdCandidatas = 0;

        //laço para percorrer cidades vizinhas
        for(j = 0; j < qtdCidades; ++j)
        {
            //checa se a cidade 'j' não foi visitada
            if(foiVisitada[j] == 0)
            {
                //define cidade 'j' como candidata a ser visitada
                ++qtdCandidatas;
                candidatas = (Candidato*) realloc(candidatas, qtdCandidatas * sizeof(Candidato));
                candidatas[qtdCandidatas - 1].indice = j;
            }
        }

        //calcula a probabilidade de cada cidade candidata ser visitada
        calculaProbabilidades(candidatas, rota[i - 1], p, feromonio, distancias, qtdCandidatas);

        //sorteia a próxima cidade a ser visitada com base nas probabilidade
        rota[i] = sorteiaProximoVertice(candidatas, qtdCandidatas);
        free(candidatas);
        candidatas = NULL;

        //incrementa custo da rota
        *custo += distancias[rota[i-1]][rota[i]];

        //marca cidade sorteada como visitada
        foiVisitada[rota[i]] = 1;
    }

    *custo += distancias[rota[0]][rota[qtdCidades - 1]];

    free(foiVisitada);
    free(candidatas);
}

void calculaProbabilidades(Candidato *candidatas, int cidadeAtual, Parametros p,
        double **feromonio, double **distancias, int qtdCandidatas)
{
    int j;
    double somatorio = 0;

    for(j = 0; j < qtdCandidatas; ++j)
    {
        somatorio += pow(feromonio[cidadeAtual][candidatas[j].indice], p.alpha) *
                pow(1/distancias[cidadeAtual][candidatas[j].indice], p.beta);
    }

    for(j = 0; j < qtdCandidatas; ++j)
    {
        candidatas[j].probabilidade = pow(feromonio[cidadeAtual][candidatas[j].indice], p.alpha) *
                pow(1/distancias[cidadeAtual][candidatas[j].indice], p.beta)/somatorio;
    }
}

int sorteiaProximoVertice(Candidato *candidatas, int qtdCandidatas)
{
    int i, sorteada;
    double probAcumulada, marcadorRoleta;

    quickSort(candidatas, qtdCandidatas);
    marcadorRoleta = ((double)rand()/(double)RAND_MAX);

    i = 0;
    sorteada = -1;
    probAcumulada = 0;

    while(sorteada == -1)
    {
        probAcumulada += candidatas[i].probabilidade;
        if(marcadorRoleta <= probAcumulada || i == qtdCandidatas - 1)
        {
            sorteada = candidatas[i].indice;
        }
        ++i;
    }

    return sorteada;
}

void quickSort(Candidato *candidatas, int n)
{
    ordena(candidatas, 0, n-1);
}

void ordena(Candidato *candidatas, int esq, int dir)
{
    int i, j;
    particiona(candidatas, esq, dir, &i, &j);
    if(esq < j) ordena(candidatas, esq, j);
    if(i < dir) ordena(candidatas, i, dir);
}

void particiona(Candidato *candidatas, int esq, int dir, int *i, int *j)
{
    Candidato aux;
    double pivo;
    *i = esq;
    *j = dir;

    pivo = candidatas[(*i + *j)/2].probabilidade;

    do
    {
        while(candidatas[*i].probabilidade < pivo)
            (*i)++;
        while(candidatas[*j].probabilidade > pivo)
            (*j)--;
        if(*i <= *j)
        {
            aux = candidatas[*i];
            candidatas[*i] = candidatas[*j];
            candidatas[*j] = aux;

            (*i)++;
            (*j)--;
        }
    }
    while(*i <= *j);
}
