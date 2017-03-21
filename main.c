#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "antSystem.h"
#include "tspReader.h"

static double const BETA = 5;
static double const ALPHA = 1;
static double const TAXA_EV = 0.5;
static double const FERO_IN = 1;
static int const ITERACOES = 100;

void solucaoPadrao(int *rota, int qtdCidades, double **distancias, Parametros p);
void solucaoMultipla(int *rota, int qtdCidades, double **distancias, Parametros p);
void calculaEst(double *valores, int n, double *media, double *variancia);
void inicializaVariaveis(Parametros *p, int *popAutomatica, int qtdCidades);
void exibeParametros(Parametros p, int popAutomatica);
void carregaInstanciaTSP(double ***distancias, int *qtdCidades, int **rota, int *populacao,
        int popAutomatica, char *nomeArquivo);
void criaInstanciaTSP(double ***distancias, int *qtdCidades, int **rota, int *populacao,
        int popAutomatica, char *nomeArquivo);
void configuraParametros(Parametros *p, int *popAutomatica, int qtdCidades);
void mostraDistancias(double **distancias, int qtdCidades);
void pause();

int main()
{
    int *rota, popAutomatica, qtdCidades, opcao, i;
    double **distancias;
    char nomeArquivo[100];
    Parametros p;

    do
    {
        do
        {
            printf("------------ TSP usando o Ant System ------------\n\n");
            printf("Opcoes disponiveis:\n\n");
            printf("  0 - Carregar arquivo de instancia;\n");
            printf("  1 - Encerrar programa;\n\n");
            printf("Opcao escolhida: ");
            scanf("%d", &opcao);

            switch(opcao)
            {
            case 0:
                system("tput reset");
                carregaInstanciaTSP(&distancias, &qtdCidades, &rota,
                        &p.populacao, popAutomatica, nomeArquivo);
                break;
            case 1:
                system("tput reset");
                break;
            default:
                system("tput reset");
                printf("Opcao invalida!");
                pause();
                system("tput reset");
                break;
            }

        }
        while(opcao != 0 && opcao != 1);

        inicializaVariaveis(&p, &popAutomatica, qtdCidades);
        system("tput reset");

        if(opcao != 1)
        {
            do
            {
                printf("------------ TSP usando o Ant System ------------\n\n");
                printf("Instancia carregada:\n\n");
                printf("    - Arquivo: %s\n    - Numero de cidades: %d\n\n", nomeArquivo, qtdCidades);
                printf("-------------------------------------------------\n\n");
                exibeParametros(p, popAutomatica);
                printf("\n-------------------------------------------------\n\n");
                printf("Opcoes disponiveis:\n\n");
                printf("  0 - Executar algoritmo (execucao padrao);\n");
                printf("  1 - Executar algoritmo (execucao multipla);\n");
                printf("  2 - Alterar parametros;\n");
                printf("  3 - Mostrar distancias;\n");
                printf("  4 - Fechar instancia;\n\n");

                printf("Opcao escolhida: ");
                scanf("%d", &opcao);

                switch(opcao)
                {
                case 0:
                    system("tput reset");
                    solucaoPadrao(rota, qtdCidades, distancias, p);
                    system("tput reset");
                    break;
                case 1:
                    system("tput reset");
                    solucaoMultipla(rota, qtdCidades, distancias, p);
                    system("tput reset");
                    break;
                case 2:
                    system("tput reset");
                    configuraParametros(&p, &popAutomatica, qtdCidades);
                    system("tput reset");
                    break;
                case 3:
                    system("tput reset");
                    mostraDistancias(distancias, qtdCidades);
                    system("tput reset");
                    break;
                case 4:
                    free(rota);
                    for(i = 0; i < qtdCidades; ++i)
                    {
                        free(distancias[i]);
                    }
                    system("tput reset");
                    break;
                default:
                    system("tput reset");
                    printf("Opcao invalida!");
                    pause();
                    system("tput reset");
                    break;
                }
            }

            while(opcao != 4);
        }
    }
    while(opcao != 1);

    return 0;
}

void solucaoPadrao(int *rota, int qtdCidades, double **distancias, Parametros p)
{
    int i;
    double tempo, custo;
    clock_t t0 = clock();
    executaAlgoritmo(p, qtdCidades, distancias, rota, &custo);
    tempo = (clock() - t0)/(double)CLOCKS_PER_SEC;

    printf("Solucao gerada com sucesso!\n\nCusto: %lf\nTempo(s): %lf\n",
           custo,tempo);
    printf("\nCaminho:\n");
    for(i = 0; i < qtdCidades; ++i)
        printf(" - [%d] \n", rota[i]);
    printf("\n");
    pause();
}
void solucaoMultipla(int *rota, int qtdCidades, double **distancias, Parametros p)
{
    int *caminhoMenorCusto, n, i, j, cont;
    double *valores, menorCusto, tempoMenorCusto, tempo, media, variancia;
    printf("Numero de solucoes a serem geradas: ");
    scanf("%d", &n);
    valores = (double*) malloc(n * sizeof(double));
    if(valores == NULL)
    {
        exit(1);
    }

    caminhoMenorCusto = (int*) malloc(qtdCidades * sizeof(int));
    if(caminhoMenorCusto == NULL)
    {
        exit(1);
    }

    clock_t t0 = clock();
    executaAlgoritmo(p, qtdCidades, distancias, rota, &valores[0]);
    tempoMenorCusto = (clock() - t0)/(double)CLOCKS_PER_SEC;
    menorCusto = valores[0];
    for(i = 0; i < qtdCidades; ++i)
        caminhoMenorCusto[i] = rota[i];

    for(i = 1; i < n; ++i)
    {
        t0 = clock();
        executaAlgoritmo(p, qtdCidades, distancias, rota, &valores[i]);
        tempo = (clock() - t0)/(double)CLOCKS_PER_SEC;

        if(valores[i] < menorCusto)
        {
            menorCusto = valores[i];
            tempoMenorCusto = tempo;
            for(j = 0; j < qtdCidades; ++j)
                caminhoMenorCusto[j] = rota[j];
        }
    }

    cont = 0;
    for(i = 0; i < n; ++i)
    {
        if(valores[i] == menorCusto)
        {
            ++cont;
        }
    }

    printf("\nSolucoes geradas com sucesso!\n\n");
    printf("Solucao de menor custo:\n");
    printf("\n   Custo: %lf\n   Tempo(s): %lf\n   Ocorrencia: %d\n",
           menorCusto, tempoMenorCusto, cont);
    printf("\n   Caminho:\n");
    for(i = 0; i < qtdCidades; ++i)
        printf("    - [%d] \n", caminhoMenorCusto[i]);
    printf("\n");

    calculaEst(valores, n, &media, &variancia);
    printf("Estatisticas das %d solucoes:\n", n);
    printf(" - Media: %f\n", media);
    printf(" - Desvio padrao: %f\n\n", sqrt(variancia));

    free(caminhoMenorCusto);
    free(valores);

    pause();
}

void calculaEst(double *valores, int n, double *media, double *variancia){
    int i;

    *media = 0;
    *variancia = 0;

    for(i = 0; i < n; ++i)
        *media += valores[i];
    *media /= n;

    for(i = 0; i < n; ++i)
        *variancia += (valores[i]-*media)*(valores[i]-*media);
    *variancia /= n;
}

void inicializaVariaveis(Parametros *p, int *popAutomatica, int qtdCidades){

    *popAutomatica = 1;

    p->beta = BETA;
    p->alpha = ALPHA;
    p->taxaEvapo = TAXA_EV;
    p->feroInicial = FERO_IN;
    p->iteracoes = ITERACOES;
    p->populacao = qtdCidades;
}

void exibeParametros(Parametros p, int popAutomatica){
    printf("Parametros atuais:\n\n");
    printf("    - Param Beta: %.2f;\n", p.beta);
    printf("    - Param Alpha: %.2f;\n", p.alpha);
    printf("    - Taxa Evap: %.2f;\n", p.taxaEvapo);
    printf("    - Fero Inicial: %.2f;\n", p.feroInicial);
    printf("    - Num Iteracoes: %d;\n", p.iteracoes);
    printf("    - Populacao: %d", p.populacao);
    if(popAutomatica)
        printf(" (Numero de cidades);\n");
    else
        printf(";\n");
}

void carregaInstanciaTSP(double ***distancias, int *qtdCidades, int **rota,
        int *populacao, int popAutomatica, char *nomeArquivo)
{
    FILE *f;
    TspInfo *tspInfo;

    printf("Informe o nome do arquivo (max 100 caracteres): ");
    scanf("%s", nomeArquivo);
    f = fopen(nomeArquivo, "r");
    tspInfo = read(f);
    fclose(f);

    *distancias = tspInfo->distances;
    *qtdCidades = tspInfo->dimension;

    if(popAutomatica == 1)
        *populacao = *qtdCidades;

    *rota = (int*) malloc(sizeof(int) * (*qtdCidades));
    if(*rota == NULL)
    {
        exit(1);
    }
}

void configuraParametros(Parametros *p, int *popAutomatica, int qtdCidades){
    int opcao;
    do{
        printf("Opcoes disponiveis:\n\n");
        printf("  0 - Alterar parametro beta (%.2lf);\n", p->beta);
        printf("  1 - Alterar parametro alpha (%.2lf);\n", p->alpha);
        printf("  2 - Alterar taxa de evaporacao (%.2lf);\n", p->taxaEvapo);
        printf("  3 - Alterar feromonio inicial (%.2lf);\n", p->feroInicial);
        printf("  4 - Alterar numero de iteracoes (%d);\n", p->iteracoes);
        printf("  5 - Alterar populacao (%d)", p->populacao);
        if(*popAutomatica)
            printf(" (Numero de cidades);\n");
        else
            printf(";\n");
        printf("  6 - Restaurar valores recomendados;\n");
        printf("  7 - Voltar ao menu principal;\n");
        printf("\nOpcao escolhida: ");
        scanf("%d", &opcao);
        system("tput reset");

        switch(opcao)
        {
        case 0:
            printf("Alterar parametro beta:\n\n");
            printf("   Valor atual: %.2f\n   Novo valor: ", p->beta);
            scanf("%lf", &p->beta);
            break;
        case 1:
            printf("Alterar parametro alpha:\n\n");
            printf("   Valor atual: %.2f\n   Novo valor: ", p->alpha);
            scanf("%lf", &p->alpha);
            break;
        case 2:
            printf("Alterar taxa de evaporacao:\n\n");
            printf("   Valor atual: %.2f\n   Novo valor: ", p->taxaEvapo);
            scanf("%lf", &p->taxaEvapo);
            break;
        case 3:
            printf("Alterar feromonio inicial:\n\n");
            printf("   Valor atual: %.2f\n   Novo valor: ", p->feroInicial);
            scanf("%lf", &p->feroInicial);
            break;
        case 4:
            printf("Alterar numero de iteracoes:\n\n");
            printf("   Valor atual: %d\n   Novo valor: ", p->iteracoes);
            scanf("%d", &p->iteracoes);
            break;
        case 5:
            do{
                printf("Alterar populacao:\n\n");
                printf("  0 - Definir valor;\n");
                printf("  1 - Manter valor igual ao numero de cidades;");
                printf("\n\nOpcao escolhida: ");
                scanf("%d", &opcao);

                switch(opcao)
                {
                case 0:
                    system("tput reset");
                    printf("Informe o valor: ");
                    scanf("%d", &p->populacao);
                    *popAutomatica = 0;
                    printf("\n");
                    break;
                case 1:
                    system("tput reset");
                    printf("    - Populacao definida de acordo com numero de vertices do grafo atual.\n\n");
                    p->populacao = qtdCidades;
                    *popAutomatica = 1;
                    pause();
                    break;
                default:
                    printf("\nOpcao invalida!\n\n");
                    pause();
                    break;
                }
            }
            while(opcao != 0 && opcao != 1);
            system("tput reset");
            break;
        case 6:
            inicializaVariaveis(p, popAutomatica, qtdCidades);
            printf("Valores restaurados!\n\n");
            pause();
            break;
        case 7:
            break;
        default:
            printf("\nOpcao invalida!\n\n");
            pause();
            break;
        }

        system("tput reset");

    }while(opcao != 7);
}

void mostraDistancias(double **distancias, int qtdCidades){
    int i, j;

    for(i = 0; i < qtdCidades; ++i)
    {
        for(j = i + 1; j < qtdCidades; ++j)
        {
            printf("[%d][%d] = %.2lf\n", i, j, distancias[i][j]);
        }
    }
    pause();
}

void pause() {

    printf("\nPressione enter para continuar");
    setbuf(stdin, NULL);
    getchar();

}
