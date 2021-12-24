// David Gonçalves Mendes - 190056967
// Projeto Final: Carrinho controlado pelo celular

#include <msp430.h>

#define TRUE 1
#define FALSE 0

#define SOBE    1   // Capturando flanco de subida
#define DESCE   0   // Capturando flanco de descida
#define TOPO 65535  // Topo da contagem = 50ms
#define PASSO 6553  // Passo de 10%
#define CR  0xD     // ASCII para Carriage return: Voltar para o inicio da linha
#define LF  0xA     // ASCII para Line Feed: Quebra de linha
#define RE     0
#define FRENTE 1

void gpio_config(void);
void ta0_config(void);
void ta1_config(void);
void ta2_config(void);
void tb0_config(void);

void both_engines(void);
void left_engine(void);
void right_engine(void);
void stop_engines(void);

void uart0_config(void);
void ser_char(char c);
void ser_str(char *p);
void ser_comandos(void);

volatile long int x, y, dif;
volatile int modo = FRENTE, sentido, flag = FALSE, dist, stop = TRUE;

void main(void){
    volatile char comando;
    volatile int velocidade = 5;

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    gpio_config();
    ta0_config();
    ta1_config();
    ta2_config();
    tb0_config();
    uart0_config();

    __enable_interrupt();

    __delay_cycles(500000);
    ser_comandos();

    while (TRUE){
        if(dist > 15){
            if((UCA0IFG&UCRXIFG)==0);   // Checar se recebeu
            comando = UCA0RXBUF;        // Buffer de recepcao
            switch(comando){
            case 'h':
                ser_comandos();
                break;
            case 'L':
                left_engine();
                break;
            case 'R':
                right_engine();
                break;
            case 'S':
                stop = TRUE;
                stop_engines();
                ser_str("Carrinho parado!\n");
                break;
            case 'M':
                stop = FALSE;
                modo = FRENTE;
                both_engines();
                ser_str("Modo: Frente\n");
                break;
            case 'm':
                modo = RE;
                both_engines();
                ser_str("Modo: Re\n");
                ser_char(CR);
                break;
            case 'A':
                if (++velocidade < 10){
                    TA2CCR2 += PASSO;
                    TB0CCR6 += PASSO;
                }
                else{
                    ser_str("Velocidade maxima atingida!");
                    velocidade--;
                }
                ser_str("Velocidade configurada: ");
                ser_char(velocidade + 0x30);
                ser_char(CR);
                ser_char(LF);
                break;
            case 'a':
                if (--velocidade > 2){
                    TA2CCR2 -= PASSO;
                    TB0CCR6 -= PASSO;
                }
                else{
                    ser_str("Velocidade minima atingida!");
                    velocidade++;
                }
                ser_str("Velocidade configurada: ");
                ser_char(velocidade + 0x30);
                ser_char(CR);
                ser_char(LF);
                break;
            case '<':
                if (velocidade < 9)
                    TA2CCR2 += PASSO;
                else
                    TB0CCR6 -= PASSO;
                break;
            case '>':
                if (velocidade < 9)
                    TB0CCR6 += PASSO;
                else
                    TA2CCR2 -= PASSO;
                break;
            case 'N':
                TA2CCR2 = velocidade*PASSO;
                TB0CCR6 = velocidade*PASSO;
                ser_str("Velocidade configurada: ");
                ser_char(velocidade + 0x30);
                ser_char(CR);
                ser_char(LF);
                break;
            }

        }else if (stop == FALSE){
            stop_engines();
            __delay_cycles(1000000);
            modo = RE;
            both_engines();
            __delay_cycles(1000000);
            right_engine();
            __delay_cycles(1000000);
            stop_engines();
            flag = FALSE;
            ser_str("Impacto evitado!\n");
            ser_comandos();
        }
    }
}

//#pragma vector = TIMER1_A1_VECTOR
#pragma vector = 48
__interrupt void ta1(void){
    TA1IV;                          // Desabilitar flag de interrupcao
    if(sentido == DESCE){           // Capturou flanco de descida?
        TA1CCTL1 &= ~CM_3;          // Zerar campo CM
        TA1CCTL1 |= CM_1;           // Prep flanco subida
        sentido = SOBE;             // Informando que a proxima borda sera de subida
        y = TA1CCR1;                // Numero de contagens ate a borda de descida
        dif = y - x;                // Tamanho do echo
        if (dif < 0)                // Caso em que y < x (Timer chegou em 0xFFFF)
            dif += 65536L;
        dist = (17*dif) >> 10;      // Calculo da distancia
        flag = TRUE;                // Informando que o calculo da distancia foi efetuado
    } else {
        x = TA1CCR1;                // Numero de contagens ate a borda e subida
        TA1CCTL1 &= ~CM_3;          // Zerar campo CM
        TA1CCTL1 |= CM_2;           // Prep flanco descida
        sentido = DESCE;            // Informando que a proxima borda sera de descida
    }

}

void both_engines(void) {
    P8OUT &= modo == FRENTE ? ~BIT1 : ~BIT2;
    P8OUT |= modo == FRENTE ?  BIT2 :  BIT1;
    P4OUT &= modo == FRENTE ? ~BIT1 : ~BIT2;
    P4OUT |= modo == FRENTE ?  BIT2 :  BIT1;
}

void left_engine(void) {
    P8OUT |= BIT2|BIT1;
    P4OUT &= modo == FRENTE ? ~BIT1 : ~BIT2;
    P4OUT |= modo == FRENTE ?  BIT2 :  BIT1;
}

void right_engine(void){
    P4OUT |= BIT2|BIT1;
    P8OUT &= modo == FRENTE ? ~BIT1 : ~BIT2;
    P8OUT |= modo == FRENTE ?  BIT2 :  BIT1;
}

void stop_engines(void){
    P8OUT |= BIT2|BIT1;
    P4OUT |= BIT2|BIT1;
}

void ser_char(char c){
    UCA0TXBUF = c;
    while((UCA0IFG&UCTXIFG)==0); // Esperar transmitir
}

void ser_str(char *p){
    unsigned int i = 0;
    while (p[i] != 0){
        ser_char(p[i]);
        i++;
    }
}

void ser_comandos(void){
    ser_str("Carrinho muito maneiro!\n");
    ser_str("Comandos:\nL - Apenas motor esquerdo\nR - Apenas o motor direito\n");
    ser_str("> - Curva para a direita\n< - Curva para a esquerda\n");
    ser_str("M - Modo Frente\nR - Modo re\nS - Parar carrinho\nN - Reseta as velocidades dos motores)\n");
    ser_str("A - Acelerar\na - Desacelerar\nh - exibe essa mensagem com os comandos.");
}

// ACLK/9.600bps => BRW = 3, UCBRS = 3, UCBRF = 0 e UCOS16 = 0
void uart0_config(void){
    UCA0CTL1 = UCSWRST; //RST=1 para USCI_A0
    UCA0CTL0 = 0; //sem paridade, 8 bits, 1 stop, modo UART
    UCA0BRW = 3; //Divisor
    UCA0MCTL = UCBRS_3; //Modulador = 3 e UCOS=0
    P3SEL |= BIT4 | BIT3; //Disponibilizar pinos
    UCA0CTL1 = UCSSEL_1; //RST=0, ACLK
}

// Trigger do ultrassom
void ta0_config(void){
    TA0CTL = TASSEL_2 | MC_1;   // Modo UP
    TA0CCR0 = TOPO;
    TA0CCTL4 = OUTMOD_6 | OUT;
    TA0CCR4 = 21;               // Larg Trig = 20 us
}

void ta1_config(void){
    TA1CTL = TASSEL_2 | MC_2;   // SMCLK | Modo continuo
    TA1CCTL1 = CM_1 | SCS |     // Flanco subida | Cap. Sincrona
               CAP | CCIE;      // Captura | Hab. Interrupt
    sentido = SOBE;
}

// PWM Motores
void ta2_config(void){
    TA2CTL = TASSEL_2 | MC_1;
    TA2CCR0 = TOPO;

    // EnA
    TA2CCTL2 = OUTMOD_6 | OUT; // Saida no modo Toggle/set
    TA2CCR2 = TA2CCR0/2;
}

void tb0_config(void){
    TB0CTL = TBSSEL_2 | MC_1;
    TB0CCR0 = TOPO;

    TB0CCTL6 = OUTMOD_6 | OUT;
    TB0CCR6 = TOPO/2;               // 50%
}

void gpio_config(void){
    // Motores:
    // P2.5 -> enA; P2.5 -> enB;
    // Enables: P2.5 = TA2.2 (A), P2.4 = TA2.1 (B)
    P2DIR |= BIT5;
    P2SEL |= BIT5;

    P3DIR |= BIT6;
    P3SEL |= BIT6;

    P4DIR |= BIT2|BIT1;            // P4.2 e P4.1 Sentido da rotacao Motor A
    P4OUT |= BIT2|BIT1;            // Motor inicialmente parado

    P8DIR |= BIT2|BIT1;            // P8.2 e P8.1 Sentido da rotacao
    P8OUT |= BIT2|BIT1;            // Motor inicialmente parado

    // Ultrassom
    P1DIR |= BIT5;  // P1.5=saida
    P1SEL |= BIT5;  // P1.5 = TA0.4

    P2DIR &= BIT0;  // P2.0 para captura
    P2SEL |= BIT0;  // P2.0 = TA1.1

    P2DIR |= BIT5;  // P2.5 Saida
    P2SEL |= BIT5;  // P2.5 = TA2.2
}
