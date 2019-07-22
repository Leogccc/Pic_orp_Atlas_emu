/* 
 * File:   pic_orp_atlas_emu.h
 * Author: Leonardo
 *
 * Created on 17 de Junho de 2019, 16:35
 */


#ifndef PIC_ORP_ATLAS_EMU_H
#define	PIC_ORP_ATLAS_EMU_H


// #define debug /*depuracao*/

#use delay(internal=32MHZ/*,restart_wdt*/) // CPU rodando em 32MHZ (clock interno); WDT sempre é resetado duranto o uso das funcoes delay built in se tiver restart_wdt como argumento

// configuracoes de hardware do PIC (fuses)
#fuses PUT, RSTOSC_HFINTRC // POR
#fuses WRT, PROTECT, NOLVP, CPD // Proteção das Memórias  
#fuses BROWNOUT,BORV27 // BOR- Brown out ativo com V_brow_out= 2.7 V (abaixo de V_brow_out ele fica no Reset)
#fuses WDT_NOSL // WhatDogTimer (WTD)
#fuses NOMCLR // MASTER CLEAR (MCLR) 

// Descrição fuses do PIC16F18326 que estão configurados 
/* 
PUT             // Power Up Timer
RSTOSC_HFINTRC   // On Power-up clock running from HFINTRC
 * 
WRT            //  Program Memory Write Protected
PROTECT        //  Code protected from reads
CPD             // Data EEPROM Code Protected
NOLVP           // No low voltage programing, B3(PIC16) or B5(PIC18) used for I/O
 * 
NOMCLR           //Master Clear pin used for I/O
 * 
WDT_NOSL //  Watch Dog Timer enable, except during SLEEP
 * 
*/ 


/*
 Def para acesso dos registradores do PIC (ambiente CCS) ; Obs: Alterar alguns desses bits do registrador(diretamente) pode sobreescrever os FUSES se não tomar cuidado
 */
#byte TRISA= getenv("SFR:TRISA") // registrador que define se os pinos do PORTA são Digital input ou  Digital output
#bit  TRISA1= TRISA.1 


#byte PORTA= getenv("SFR:PORTA") // registrador para leitura do estado atual dos pinos do PORTA (pode ser usado para escrita, igual ao LATA)
#bit  PORTA1= PORTA.1 

#byte LATA=getenv("SFR:LATA") // registrador que altera as saídas nos pinos digitais do PORTA(caso seja input muda a semântica desse registrador)
#bit  LATA1= LATA.1 
///////////////////////

#byte TRISC= getenv("SFR:TRISC") // registrador que define se os pinos do PORTC são Digital input ou  Digital output
#bit  TRISC2= TRISC.2 
#bit  TRISC3= TRISC.3


#byte PORTC= getenv("SFR:PORTC") // registrador para leitura do estado atual dos pinos do PORTC (pode ser usado para escrita, igual ao LATC)
#bit  PORTC2= PORTC.2 
#bit  PORTC3= PORTC.3 

#byte LATC= getenv("SFR:LATC") // registrador que altera as saídas nos pinos digitais do PORTC(caso seja input muda a semântica desse registrador)
#bit  LATC2= LATC.2 
#bit  LATC3= LATC.3 




#byte FVRCON =getenv("SFR:FVRCON") // FIXED VOLTAGE REFERENCE CONTROL REGISTER

/* bit 7 FVREN: Fixed Voltage Reference Enable bit bit 7 FVREN: Fixed Voltage Reference Enable bit
1 = Fixed Voltage Reference is enabled
0 = Fixed Voltage Reference is disabled
*/
#bit  FVREN = FVRCON.7 
   
/*
bit 5 TSEN: Temperature Indicator Enable bit(3)
1 = Temperature Indicator is enabled
0 = Temperature Indicator is disabled
*/
#bit  TSEN = FVRCON.5
/*
bit 4 TSRNG: Temperature Indicator Range Selection bit(3)
1 => VOUT = VDD - 4VT (High Range)
0 => VOUT = VDD - 2VT (Low Range)
 * 
 Para TEMPERATURE INDICATOR MODULE
 TSRNG = 1 => Min. VDD  3.6V
 TSRNG = 0 => Min. VDD, 1.8V
/////////////////////////////////////////////////////////////////////////////
*/
#bit  TSRNG = FVRCON.4

/*
bit 1-0 ADFVR<1:0>: ADC FVR Buffer Gain Selection bit
11 = ADC FVR Buffer Gain is 4x, (4.096V)(2)
10 = ADC FVR Buffer Gain is 2x, (2.048V)(2)
01 = ADC FVR Buffer Gain is 1x, (1.024V)
00 = ADC FVR Buffer is off
*/
#bit ADFVR_bit1= FVRCON.1
#bit ADFVR_bit0= FVRCON.0

// REGISTRADORES PARA DESABILITAR\HABILITAR OS MÓDULOS DO PIC ( all modules are ON by default following any Reset.) : (CAP 14 datasheet)


#byte PMD0 = getenv("SFR:PMD0") //PMD0: PMD CONTROL REGISTER 0
/*
bit 7 SYSCMD: Disable Peripheral System Clock Network bit
See description in Section 14.3 ?System Clock Disable?.
1 = System Clock network disabled (a.k.a. FOSC)
0 = System Clock network enabled
*/
#bit  SYSCMD = PMD0.7
/*
bit 6 FVRMD: Disable Fixed Voltage Reference FVR bit
1 = FVR module disabled
0 = FVR module enabled
*/
#bit  FVRMD = PMD0.6
// bit 5-3 Unimplemented: Read as ?0?

 /* bit 2 NVMMD: NVM Module Disable bit(1)
1 = Data EEPROM (a.k.a. user memory, EEPROM) reading and writing is disabled; NVMCON
registers cannot be written; FSR access to EEPROM returns zero.
0 = NVM module enabled
*/
#bit NVMMD = PMD0.2

 /* bit 1 CLKRMD: Disable Clock Reference CLKR bit
1 = CLKR module disabled
0 = CLKR module enabled */
#bit CLKRMD = PMD0.1

/* bit 0 IOCMD: Disable Interrupt-on-Change bit, All Ports
1 = IOC module(s) disabled
0 = IOC module(s) enabled */
#bit IOCMD = PMD0.0

// Note 1: When enabling NVM, a delay of up to 1 ?s may be required before accessing data.
////////////////////////////////////////////////////////

#byte PMD1 = getenv("SFR:PMD1") //PMD1: PMD CONTROL REGISTER 1
/* bits PMD1:
1 = module disabled
0 = module enabled */

#bit NCOMD =  PMD1.7 //bit 7 NCOMD: Disable Numerically Control Oscillator bit
#bit TMR6MD = PMD1.6 //bit 6 TMR6MD: Disable Timer TMR6 bit
#bit TMR5MD = PMD1.5 //bit 5 TMR5MD: Disable Timer TMR5 bit
#bit TMR4MD = PMD1.4 //bit 4 TMR4MD: Disable Timer TMR4 bit
#bit TMR3MD = PMD1.3 //bit 3 TMR3MD: Disable Timer TMR3 bit
#bit TMR2MD = PMD1.2 //bit 2 TMR2MD: Disable Timer TMR2 bit
#bit TMR1MD = PMD1.1 //bit 1 TMR1MD: Disable Timer TMR1 bit
#bit TMR0MD = PMD1.0 //bit 0 TMR0MD: Disable Timer TMR0 bit

#byte PMD2 =  getenv("SFR:PMD2") //PMD2: PMD CONTROL REGISTER 2
/* bits PMD2:
1 = module disabled
0 = module enabled */

//bit 7 Unimplemented: Read as ?0?
#bit DACMD = PMD2.6 //bit 6 DACMD: Disable DAC bit
#bit ADCMD = PMD2.5 //bit 5 ADCMD: Disable ADC bit
//bit 4-3 Unimplemented: Read as ?0?
#bit CMP2MD = PMD2.2 //bit 2 CMP2MD: Disable Comparator C2 bit
#bit CMP1MD = PMD2.1 //bit 1 CMP1MD: Disable Comparator C1 bit
//bit 0 Unimplemented: Read as ?0?


#byte PMD3 = getenv("SFR:PMD3") //PMD3: PMD CONTROL REGISTER 3
/* bits PMD3:
1 = module disabled
0 = module enabled */

#bit CWG2MD = PMD3.7     //bit 7 CWG2MD: Disable CWG2 bit
#bit CWG1MD = PMD3.6     //bit 6 CWG1MD: Disable CWG1 bit
#bit PWM6MD = PMD3.5     //bit 5 PWM6MD: Disable PWM6 bit
#bit PWM5MD = PMD3.4     //bit 4 PWM5MD: Disable PWM5 bit
#bit CCP4MD = PMD3.3     //bit 3 CCP4MD: Disable CCP4 bit
#bit CCP3MD = PMD3.2     //bit 2 CCP3MD: Disable CCP3 bit
#bit CCP2MD = PMD3.1     //bit 1 CCP2MD: Disable CCP2 bit
#bit CCP1MD = PMD3.0     //bit 0 CCP1MD: Disable CCP1 bit

#byte PMD4 = getenv("SFR:PMD4") //PMD4: PMD CONTROL REGISTER 4
/* bits PMD4:
1 = module disabled
0 = module enabled */

//bit 7-6 Unimplemented: Read as ?0?
#bit UART1MD = PMD4.5  //bit 5 UART1MD: Disable EUSART1 bit
//bit 4-3 Unimplemented: Read as ?0?
#bit MSSP2MD = PMD4.2     //bit 2 MSSP2MD: Disable MSSP2 bit
#bit MSSP1MD = PMD4.1     //bit 1 MSSP1MD: Disable MSSP1 bit
//bit 0 Unimplemented: Read as ?0?


#byte PMD5 = getenv("SFR:PMD5") //PMD5: PMD CONTROL REGISTER 5
/* bits PMD5:
1 = module disabled
0 = module enabled */
//bit 7-5 Unimplemented: Read as ?0?
#bit CLC4MD = PMD5.4   //bit 4 CLC4MD: Disable CLC4 bit
#bit CLC3MD = PMD5.3   //bit 3 CLC3MD: Disable CLC3 bit
#bit CLC2MD = PMD5.2    //bit 2 CLC2MD: Disable CLC2 bit
#bit CLC1MD = PMD5.1     //bit 1 CLC1MD: Disable CLC1 bit
#bit DSMMD = PMD5.0      //bit 0 DSMMD: Disable Data Signal Modulator bit


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define VERSAO_FIRMWARE 1.0
#define BUF_SIZE 40 // tamanho dos Buffers do I2C1
#define PIC_ADDRESS  0xC4// endereco padrão da I2C1 do pic Modo Slave (endereco em 7 bits) 0x62; As funcoes CCS usam endereco na forma 8 bits 0x62<<1= 0xC4
#define LED_PIN PIN_A1 // pino onde está conectado o LED pino 12 RA1
#define LED_STATUS  PIN_A1
#define TAM_MAX  25

#define LEN_MAX_CMD 7 // o numero maximo de caracteres dos comandos da lista_comandos  ( não contando '\0') para comandos da forma : "CMD"
#define LEN_MAX_CMD2 5  // tamanho max do cmd2 ( não contando '\0') para um comando da forma CMD,CMD2,VALOR
#define LEN_MAX_VALOR 28 // tamanho max de  valor (não contando '\0') para comandos da forma CMD,VALOR ouu CMD,CMD2,VALOR pode variar, mas não passara de 20

// Macros de reset dos buffers do I2C1 (buffers não físicos)
#define RESET_out_buffer  for(int i = 0; i <= BUF_SIZE; i++) out_buffer[i] =0;  // reseta o buffer de entrada do I2C1
#define RESET_in_buffer   for(int i = 0; i <= BUF_SIZE; i++) in_buffer[i] =0;  // reseta o buffer de saida do I2C1
#define RESET_out_buffers for(int i = 0; i <= BUF_SIZE; i++) { out_buffer[i] =0; if(i<=LEN_MAX_CMD)CMD[i] =0; if(i<=LEN_MAX_CMD2) CMD2[i] =0; if(i<=LEN_MAX_VALOR)VALOR[i]=0;} // reseta os buffers de entrada e saída do PIC (modo Slave)


// associando os pinos do pic com a funcao que ele desempenha ( #pin_select) para uso da I2C1
// PPS do PIC
 #pin_select SCL1IN=PIN_C0 
 #pin_select SCL1OUT=PIN_C0 // Modo slave não controla clock
 #pin_select SDA1IN=PIN_C1
 #pin_select SDA1OUT=PIN_C1
/*configura a I2C1(MSSP1) para o modo slave, com endereco PIC_ADDRESS,  usando hardware I2C 
 functions(FORCE_HW). Uma ID= I2C_PIC_SLAVE  representa o canal(stream) i2c criado 
 A i2c é iniciado com i2c_init() (NOINIT) ,
 restart_wdt=> Restart the WDT while waiting in I2C_READ 
 */ 
#use i2c(SLAVE, I2C1, address= PIC_ADDRESS , stream= I2C_PIC_SLAVE, NOINIT,FORCE_HW, restart_wdt) 



//unsigned int8 tamanho = 0;
char in_buffer[BUF_SIZE+1]; // buffer de recepcao de dados vindos do Master para o PIC
char out_buffer[BUF_SIZE+1]; // buffer de saida dados do slave(PIC) para o master

char CMD[LEN_MAX_CMD +1]={};
char CMD2[LEN_MAX_CMD2+1]={};
char VALOR[LEN_MAX_VALOR+1]={} ;


// unsigned int8  index_in_buffer_uart =0; 

unsigned int1  lendo_str_master = FALSE ; // indica se o PIC está preenchendo in_buffer (I2C)
unsigned int8  index_out_buffer;
unsigned int8  index_in_buffer ;
unsigned int1  FIND_exe= FALSE ;// status do comando FIND (se está executando é TRUE)
unsigned int1  SLEEP_exe= FALSE ; // // status do comando SLEEP
unsigned int8  device_calibrated ; // indica se o dispositivo está calibrado ou não TRUE= Calibrado; FALSE= Não calibrado
unsigned int8  estado_led; // Estado do led controlado pelo comando L, salvo na EEPROM para voltar caso a energia acabe 
unsigned int8  i2c_address; // endereco da i2c da placa padrão é 98 (DEC),mas pode ser alterado com o comando I2C

int8 Reason_for_restart ; // indica a causa do ultimo restart do PIC


typedef enum {cmd_err, cmd_Baud ,cmd_Cal, cmd_Export, cmd_Factory, cmd_Find, cmd_i, cmd_I2C, cmd_Import, cmd_L, cmd_Plock, cmd_R, cmd_Sleep, cmd_Slope,cmd_Status} comandos;
char lista_comandos[15][LEN_MAX_CMD +1]= {"ERR","BAUD","CAL","EXPORT","FACTORY","FIND","I","I2C","IMPORT","L","PLOCK","R","SLEEP","SLOPE","STATUS"} ;

#separate
void config_PIC(void) ;

#separate
void disable_Modulos_PIC(void);

#separate
void renable_Modulos_PIC(void);

/*
 Faz o parseamento de in_buffer, conforme o protocolo definido abaixo:
 in_buffer pode ser da forma "CMD", "CMD,VALOR" ou "CMD,CMD2,VALOR"
 Monta as string CMD(comando), CMD2(comando secundário (nem todos os comandos tem um CMD2) ) e VALOR( parâmetro de algum comando)
 Obs: o separador padrão é ',' mas pode ser alterado para separador=%c
 retorna TRUE se o parseamento foi correto (nº esperado de separadores)
 retorna FALSE se o parseamento foi inválido(erro de sintaxe) (nº inesperado de separadores)
 */

#separate
int1 parsing_in_buffer(char *rcv_buffer) ; 

/* Retorna qual  comando(CMD) foi enviado pelo usuário
 O retorno é um valor de enum comandos
 */
#inline
int8 identifica_comando( char * comando) ;

#inline
void monta_out_buffer( int8 num_comando ) ; // monta o vetor out_buffer de resposta as requisicoes do mestre


/*
 Variáveis e definições ANORP
 */

 union float32_eeprom 
    {
        float32  valor; // float
        unsigned int8 valor_byte[4]; // bytes do float onde MSB valor_byte[0] e LSB é valor_byte[3]
     }  ;

union float32_eeprom offset_cal; //valor  do offset usado na calibracao do eletrodo de ORP (Salvo na eeprom)


 /*
 Protótipos das funções do ANORP
 */

#separate
int1 isStr_float(char *str_teste) ;// verifica se uma string representa um float válido

#inline 
float32 get_orp_value_mV(void) ;

#inline // é o padrao do CCS (O codigo da funcao é colocado diretamente onde ocorre a chamada) (codigo mais rapido pq não tem chamada de funcoes (economiza a pilha e consome mais rom) ) )
void ANORP_R(void) ; // retorna uma única leitura do valor de ph (%.2f) 

#inline 
void ANORP_FIND(void); //Find: LED rapidly blinks white, used to help find device

#inline 
void ANORP_i(void); // retorna device information para o usuario

#separate // isso faz com que CCS use chamadas as funcoes (uso de stack) ao invés de inline(inserir o codigo da funcao diretamente(evita overhead e uso da pilha) mas consome + ROM) economizando memoria de programa(evita passar dos limites dos segmentos da ROM (Olhar main.stat para ver os segmentos) )
void ANORP_I2C(void); //

#separate
void ANORP_L(void); // LED CONTROL

#separate
void ANORP_STATUS(void) ; //  Status voltage at Vcc pin and reason for last restart

#inline
void ANORP_SLEEP(void) ; // Placa entre em modo Sleep

#separate
void ANORP_CAL(void); //

#separate
void ANORP_FACTORY(void);//

/* */



#ifndef debug

//////////////////// Driver for MCP3421 A/D Converter ///////////////////
////                                                                 ////
////  adc_init() - initializes the MCP3421, call after power up      ////
////                                                                 ////
////  read_adc_mcp3421() - initiates a conversion and reads adc,     ////
////                       returns raw reading.                      ////
////                                                                 ////
////  read_adc_volts_mcp3421() - initiates a conversion and read     ////
////                             adc, returns voltage as float       ////
////                             (-2.048 to 2.048).                  ////
////                                                                 ////
////  Defines used to setup MCP3421:                                 ////
////                                                                 ////
////    MCP3421_SCL  - specifies the I2C clock pin to use, default   ////
////                   if not specified is PIN_C3.                   ////
////                                                                 ////
////    MCP3421_SDA  - specifies the I2C data pin to use, default if ////
////                   not specified is PIN_C4.                      ////
////                                                                 ////
////    MCP3421_MODE - specifies the mode to use, default if not     ////
////                   specified is MCP3421_CONTINUOUS.  Valid       ////
////                   options are:                                  ////
////                     MCP3421_CONTINUOUS                          ////
////                     MCP3421_ONE_SHOT                            ////
////                                                                 ////
////    MCP3421_BITS - specifies the number of conversion bits to    ////
////                   use, default if not specified is              ////
////                   MCP3421_18BITS.  Valid options are:           ////
////                     MCP3421_18BITS                              ////
////                     MCP3421_16BITS                              ////
////                     MCP3421_14BITS                              ////
////                     MCP3421_12BITS                              ////
////                                                                 ////
////    MCP3421_GAIN - specifies the PGA gain to use, default if not ////
////                   specified is MCP3421_1X_GAIN.  Valid options  ////
////                   are:                                          ////
////                     MCP3421_8X_GAIN                             ////
////                     MCP3421_4X_GAIN                             ////
////                     MCP3421_2X_GAIN                             ////
////                     MCP3421_1X_GAIN                             ////
////                                                                 ////
////    MCP3421_ADDRESS - specifies the address of MCP3421 devices,  ////
////                      default if not specified is 0.  Valid      ////
////                      options are 0-7.                           ////
////                                                                 ////
////    USE_HW_I2C - specifies that the MCP3421_SCL and MCP3421_SDA  ////
////                 pins are HW I2C pins and to use PIC's HW I2C    ////
////                 peripheral.                                     ////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2013 Custom Computer Services         ////
//// This source code may only be used by licensed users of the CCS  ////
//// C compiler.  This source code may only be distributed to other  ////
//// licensed users of the CCS C compiler.  No other use,            ////
//// reproduction or distribution is permitted without written       ////
//// permission.  Derivative programs created using this software    ////
//// in object code form are not restricted in any way.              ////
/////////////////////////////////////////////////////////////////////////

#define OFFSET_HARDWARE_mV -1.03 // esse é o valor que a placa mostra quando os terminais do probe são curto circuitados (era pra medir 0V)

#define MCP3421_CONTINUOUS 0x10
#define MCP3421_ONE_SHOT   0x00

#define MCP3421_18BITS     0x0C
#define MCP3421_16BITS     0x08
#define MCP3421_14BITS     0x04
#define MCP3421_12BITS     0x00

#define MCP3421_8X_GAIN    0x03
#define MCP3421_4X_GAIN    0x02
#define MCP3421_2X_GAIN    0x01
#define MCP3421_1X_GAIN    0x00

#define MCP3421_DEVICE_CODE       0xD0
#define MCP3421_START_CONVERSTION 0x80

#ifndef MCP3421_SCL
 #define MCP3421_SCL  PIN_C4
#endif

#ifndef MCP3421_SDA
 #define MCP3421_SDA  PIN_C5
#endif


#ifndef MCP3421_MODE
 #define MCP3421_MODE /*MCP3421_ONE_SHOT*/ MCP3421_CONTINUOUS
#endif

#ifndef MCP3421_BITS
 #define MCP3421_BITS MCP3421_18BITS
#endif

#ifndef MCP3421_GAIN
 #define MCP3421_GAIN MCP3421_1X_GAIN
#endif

#ifndef MCP3421_ADDRESS
 #define MCP3421_ADDRESS 0 // endereco MCP3421 0x68=> 0xD0|0x00<<1  ;  
#endif


 #pin_select SCL2IN = MCP3421_SCL 
 #pin_select SCL2OUT = MCP3421_SCL
 #pin_select SDA2IN = MCP3421_SDA
 #pin_select SDA2OUT = MCP3421_SDA


#use i2c(MASTER,FAST,SCL=MCP3421_SCL, SDA=MCP3421_SDA, stream= MCP3421_STREAM,NOINIT, restart_wdt)


/* Protótipos*/
#inline
void adc_init(unsigned int8 address=MCP3421_ADDRESS) ;

#if MCP3421_BITS == MCP3421_18BITS

#separate
signed int32 read_adc_mcp3421(unsigned int8 address=MCP3421_ADDRESS) ;
#else
signed int16 read_adc_mcp3421(unsigned int8 address=MCP3421_ADDRESS) ;
#endif

#separate
float32 read_adc_volts_mcp3421(unsigned int8 address=MCP3421_ADDRESS) ;

#endif // debug

#endif	/* PIC_ORP_ATLAS_EMU_H */