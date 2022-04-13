#include "sdstore.h"

//estrutura de dados para implementar o dicionário dos limites
//como determinar o número? começar com hardcode a 13

int main(int argc, char const *argv[])
{
    //o servidor é executado com o config e com a pasta
    //todo teste para ver se não nos estão a tentar executar o server maliciosamente

    char pasta[] = argv[2];
    //lembrar da pasta em algum sítio para os executáveis
    //eventualmente fazer sprintf("%s/%s", nome da pasta, nome da transforamação)

    //todo preencher o dicionário com 1º argumento
    //parse desse ficheiro .config: readln c/ sequencial até ' '

    //depois do servidor ser executado, fica à espera de ler do pipe com nome a instrução
    return 0;
}
