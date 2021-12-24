# Projeto Final - Laboratório de Sistemas Microprocessados

Esse repositório consiste na implementação de um carrinho utilizando a plaquinha MSP430F5529LP, que é um microcontrolador da Texas Instruments. Controlado via terminal serial bluetooth. Para mais detalhes à respeito do funcionamento do projeto, verifique o [relatório](carrinho-190056967.pdf).

## Compilação 

Esse projeto foi feito utilizando IDE própria da texas, o Code Composer Studio, para compilar o arquivo e gerar o executável.

## Conexão

Tenha muito cuidado ao conectar os módulos externos à placa, pois alguns necessitam de um divisor resistivo para não queimar algum pino de sua placa. Observe o arquivo [PinMap.txt](PinMap.txt) para verificar como é a conexão de cada um dos componentes. Atente-se, também, à voltagem de cada componente, pois alguns funcionam em 3,3V e outros apenas em 5V.
