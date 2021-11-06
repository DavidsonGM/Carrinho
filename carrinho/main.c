#include <msp430.h>

#define TRUE 1
#define TOPO 65535  // Topo da contagem = 50ms
#define CR  0xD // ASCII para Carriage return: Voltar para o inicio da linha
#define LF  0xA // ASCII para Line Feed: Quebra de linha
#define RE     0
#define NORMAL 1

void ta0_config(void);
void ta2_config(void);
void gpio_config(void);
void tb0_config(void);

void both_engines(void);
void both_engines_re(void);
void left_engine(void);
void right_engine(void);
void stop_engines(void);

void uart0_config(void);
void ser_char(char c);
void ser_str(char *p);

int main(void){
    volatile char comando;
    volatile int modo = NORMAL;

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    gpio_config();
    ta0_config();
    ta2_config();
    tb0_config();
    uart0_config();

    ser_str("Carrinho muito maneiro!\n");
    ser_char(CR);
    ser_str("Comandos: L, R, B, S, M, m.\n");
    ser_char(CR);

    while (TRUE){
        while((UCA0IFG&UCRXIFG)==0); // Esperar receber
            comando = UCA0RXBUF;     // Buffer de recepcao
            switch(comando){
                case 'L':
                    left_engine();
                    break;
                case 'R':
                    right_engine();
                    break;
                case 'S':
                    stop_engines();
                    break;
                case 'M':
                    both_engines();
                    ser_str("Modo: Normal\n");
                    ser_char(CR);
                    break;
                case 'm':
                    both_engines_re();
                    ser_str("Modo: Re\n");
                    ser_char(CR);
                    break;
                case 'D':       // Temporario, simular a interrupcao do ultrassom
                    stop_engines();
                    __delay_cycles(1000000);
                    both_engines_re();
                    right_engine();
                    __delay_cycles(500000);
                    both_engines();
                    break;
                }
    }
    return 0;
}

void both_engines(void) {
//    TA0CTL |= MC_1;
//    TB0CTL |= MC_1;
    P4OUT &= ~BIT1;
    P4OUT |=  BIT2;
    P2OUT &= ~BIT0;
    P2OUT |=  BIT2;
}

void both_engines_re(void) {
//    TA0CTL |= MC_1;
//    TB0CTL |= MC_1;
    P4OUT |=  BIT1;
    P4OUT &= ~BIT2;
    P2OUT |=  BIT0;
    P2OUT &= ~BIT2;
}

void left_engine(void) {
//    TA0CTL |=  MC_1;
//    TB0CTL &= ~MC_1;
    P2OUT |= BIT2|BIT0;
    P4OUT &= ~BIT1;
    P4OUT |=  BIT2;
}

void right_engine(void){
//    TA0CTL &= ~MC_1;
//    TB0CTL |=  MC_1;
    P4OUT |= BIT2|BIT1;
    P2OUT &= ~BIT0;
    P2OUT |=  BIT2;
}
void stop_engines(void){
//    TA0CTL &= ~MC_1;
//    TB0CTL &= ~MC_1;
    P4OUT |= BIT2|BIT1;
    P2OUT |= BIT2|BIT0;
}

//void toggle_engines(void){
//    P4OUT &= ~BIT1;
//    P4OUT |=  BIT2;
//    P2OUT &= ~BIT0;
//    P2OUT |=  BIT2;
//}

void ser_char(char c){
    UCA1TXBUF = c;
    while((UCA1IFG&UCTXIFG)==0); // Esperar transmitir
}

void ser_str(char *p){
    unsigned int i = 0;
    while (p[i] != 0){
        ser_char(p[i]);
        i++;
    }
}

// ACLK/9.600bps => BRW = 27, UCBRS = 2, UCBRF = 0 e UCOS16 = 0
void uart0_config(void){
    UCA0CTL1 = UCSWRST; //RST=1 para USCI_A0
    UCA0CTL0 = 0; //sem paridade, 8 bits, 1 stop, modo UART
    UCA0BRW = 3; //Divisor
    UCA0MCTL = UCBRS_3; //Modulador = 3 e UCOS=0
    P3SEL |= BIT4 | BIT3; //Disponibilizar pinos
    UCA0CTL1 = UCSSEL_1; //RST=0, ACLK
}

void ta2_config(void){
    TA2CTL = TASSEL_2 | MC_1;
    TA2CCR0 = TOPO;
    TA2CCTL2 = OUTMOD_6 | OUT; // Saida no modo Toggle/set
    TA2CCR2 = TA2CCR0/2;
}

void ta0_config(void){
    TA0CTL = TASSEL_2 | MC_1;   // Modo UP
    TA0CCR0 = TOPO;
    TA0CCTL4 = OUTMOD_6 | OUT;
    TA0CCR4 = TA0CCR0/2;
}

void tb0_config(void){
    TB0CTL = TBSSEL_2 | MC_1;
    TB0CCR0 = TOPO;

    TB0CCTL1 = OUTMOD_6 | OUT;
    TB0CCR1 = TOPO/2;               // 50%
}

void gpio_config(void){
    P1DIR |= BIT5;  //P1.5=saida
    P1SEL |= BIT5;  //P1.5 = TA0.4

    P2DIR |= BIT5|BIT2|BIT0;  // P2.5 Saida
    P2SEL |= BIT5;  // P2.5 = TA2.2
    P2OUT |= BIT2|BIT0;       // Motor inicialmente parado

    P1DIR |=  BIT0; // Led Vm
    P1OUT &= ~BIT0; // Apagado

    P4DIR |= BIT7|BIT2|BIT1; // Led Vd | P4.2 e P4.1 Sentido da rotacao
    P4SEL |= BIT7;
    PMAPKEYID = 0X02D52;   // Liberar mapeamento
    P4MAP7 = PM_TB0CCR1A;
    P4OUT |=  BIT2|BIT1;    // Motor inicialmente parado
}
