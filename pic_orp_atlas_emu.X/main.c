/* 
 * File:   main.c
 * Author: Leonardo
 *
 * Created on 17 de Junho de 2019, 16:35
 */

/*
 PIC Operating Voltage Range:
- 2.3V to 5.5V (PIC16F18326/18346)

 Inicialização:  Mantém no reset até Vdd>2.3(POR) ; Se Vdd<2.3=> Desliga PIC 
 Durante a operação(Vdd>2.3V): caso ocorra a condicao de BOR (Vdd<VBORth) o pic Reseta até que Vdd>VBORth seja verdadeiro (tem um delay para estabilizacao de Vdd)
 Modo sleep ( diminui o consumo de energia através da diminuição do consumo de corrente ( CPU e Memória ficam "paradas" 
 * ; Periféricos continuam operando) )
 
 */



#DEVICE PIC16F18326 ADC=10 //10 is the number of bits read_adc() should return 
#DEVICE PIC16F18326 CONST=ROM // Uses the CCS compiler traditional keyword CONST definition, making CONST variables located in program memory
      
#include <16F18326.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include<string.h>
#include<ctype.h>

#include "MAPA_DATA_EEPROM.h"
#include "pic_orp_atlas_emu.h"

//------------------Serial data receive interrupt------------------------------------------------- 

/*
#INT_RDA 
void  RDA_isr(void) 
{   
   if(index_in_buffer_uart<=BUF_SIZE) {
        
      
    if ( ( in_buffer[index_in_buffer_uart++]= getc() ) =='\0') 
 
    {   
        index_in_buffer_uart =0; 
        flag_monta_out_buffer=TRUE; 
        
    } 
      
   }

   else index_in_buffer_uart =0; // retorna o index no inicio do vetor (sobreescreve o BUFFER)
           
} 

*/

#INT_SSP // interrupcao I2C1 ( dispara a cada dado recebido ou dado enviado por SSP, chamando SSP_isr_slave )
void SSP_isr_slave(void) // ISR associado a interrupcao INT_SSP 
{

    
   unsigned int8 state;
   state = i2c_isr_state(I2C_PIC_SLAVE);
       
if(state == 0x00 ) /*recebou o endereco do master( bit R/W =0 escrita), slave ira armazenar os dados vindos pelo master*/
    { 
      i2c_read(I2C_PIC_SLAVE); 
      index_in_buffer=0; 
      

      if( FIND_exe==FALSE ) {lendo_str_master = TRUE ;  out_buffer[0]= 254 ; }                  // Response code: 254 still processing, not ready (exceto quando FIND está executando e está esperando um caracter da I2C para finalizar o comando)
      else out_buffer[0]= 0; // limpa a resposta anterior do FIND 
       // isso evita que o caracter de saida (lixo) do FIND ou Sleep (ou algum outro) seja analisado
           
}
   
if(state == 0x80)  { i2c_read(I2C_PIC_SLAVE,2); index_out_buffer=0 ; }// recebeu o endereco do master (bit R/W =1 leitura), slave deve responder a requisicao de leitura do master; e reinicia o indice do buffer tbm         

if(state >= 0x80) { 
    
    if(index_out_buffer<=BUF_SIZE) {
        
    // slave respondendo a requisicao do master
     i2c_write(I2C_PIC_SLAVE, out_buffer[index_out_buffer] );
     index_out_buffer++ ; 
                
    }
}

   
else {
 if(state > 0x00) { // master escrevendo em slave
    
     if(index_in_buffer<=BUF_SIZE) {
    
   
    in_buffer[index_in_buffer] = i2c_read(I2C_PIC_SLAVE) ;// o slave lê só até o byte BUF_SIZE-1 ou até receber um caracter nulo; O slave envia  um nack para o mestre na leitura desse byte(ultimo byte) informando para o mestre gerar um stop no protocolo(parar de enviar dados)) 
    index_in_buffer++ ; 
      
                                   }
     
                  }
    }
} // fim isr_slave



void main()
{   
    
    Reason_for_restart = restart_cause() ; // Usado no comando STATUS
    
    disable_Modulos_PIC(); // desabilita todos os modulos do PIC (exceto alguns que eu não estou alterando)
    renable_Modulos_PIC(); // reabilita só os modulos que eu estou utilizando e os configura (chama config_PIC() ; // configura os Módulos, Registradores etc )
    //config_PIC();
    
    unsigned int1 flag_monta_out_buffer= FALSE ;// flag para montar o vetor out_buffer
    
    while(TRUE)
    { 
      restart_wdt();
      
      // delay de leitura do in_buffer I2C
      if(lendo_str_master) { 
          set_timer0(0);// timer0 comeca a contar do 0 (incrementando a cada 512us; para um delay de 30ms=512us*N_incrementos => N_incrementos=58.59375~ 59)  
          while(get_timer0()<=59){;} // delay de ~30ms
          
          flag_monta_out_buffer= TRUE ;
          in_buffer[BUF_SIZE]='\0'; // adiciona o caracter nulo para formar a string in_buffer
          
          lendo_str_master= FALSE ; 
      } // habilita a montagem do vetor out_buffer quando tiver preenchido o buffer de entrada (recebido todo o comando do mestre )get_timer0() >=59
              
      
      if(flag_monta_out_buffer==TRUE) 
      {
            flag_monta_out_buffer=FALSE ; 
            RESET_out_buffers ; // reseta out_buffer, CMD,CMD2 e VALOR

            int1 parseamento_ok= parsing_in_buffer(in_buffer);

            if(parseamento_ok)
            {
              int8 cmd_identificado = identifica_comando(CMD) ;
              monta_out_buffer(cmd_identificado); 
              if(cmd_identificado==cmd_I2C) ANORP_I2C(); // era pra estar em monta_out_buffer mas ta com problema na ROM
            }

            // fprintf(UART_PIC,"%s", in_buffer) ;
           RESET_in_buffer ; // limpa o in_buffer (deixa limpo  para um proximo comando)
    
      } // fim if flag_monta_out_buffer  
     
    } // fim loop   
} // fim main

void config_PIC(void) 
{   
   
    setup_wdt(WDT_4S); // configura o WTD para resetar dps de 4s sem chamar restart_wdt(); Comeca a contar logo após do fuse WDT
    
    
    enable_interrupts(INT_SSP); // habilita a interrupcao de uma atividade em MSSP1 (interrupcao da I2C PIC SLAVE) 
    // enable_interrupts(INT_RDA); // habilita a interrupcao recieve data rs232
    enable_interrupts(GLOBAL); // habilita todas as interrupcoes unmasked, que foram habilitadas anteriormente dessa chamada
    
    i2c_init(I2C_PIC_SLAVE,1); // inicia I2C1 (modo slave) 
    
    i2c_init( MCP3421_STREAM,1); // inicia I2C2 (MCP3421) 
    adc_init(MCP3421_ADDRESS);// envia a configuração inicial para o MCP3421
    
 
   FVREN=1 ; // Fixed Voltage Reference is enabled  
// TSEN=1 ; // Temperature Indicator is enabled
// TSRNG= 0 ; 
         
 /*Configuracao do ADC do PIC*/
    // ADFVR<1:0> = ADC FVR Buffer Gain is 2x, (2.048V)( Tensão de referência do ADC)
    ADFVR_bit1= 1 ;
    ADFVR_bit0= 0 ;
    
    setup_adc(ADC_CLOCK_INTERNAL) ; // clock do adc é Fosc=16MHz/4
    setup_adc_ports(sAN2); // pino A2 definido como entrada analógica ; usa o canal conectado ao Vdd da placa    */ 
    set_adc_channel(2) ; // definindo de qual canal o ADC fará a leitura ( pino A2 está no canal 2 do ADC)
    setup_adc_reference(VSS_FVR); // Range 0-Fixed Voltage Reference(FVR) (0-2.048)
    // set_adc_channel(TEMPERATURE_INDICATOR); // usa o canal interno para medir Vout 
      //set_adc_channel(FVR_CHANNEL); // usa o canal interno para medir FVR    
   // setup_uart(TRUE,UART_PIC); // inicia a UART
        
    
    /* Configuracao e uso do DAC
 //setup_dac(DAC_VSS_VDD | DAC_OUTPUT);                // setup conversor digital para analógico (5 bits)
   //dac_write(4);//(5/31)*4 V                                    // Write DAC value 0-31 (5 bits)/*           
    */
            
    /*Cálculo de parametros associados a interrupcao de TMR0(por overflow)
    FCLK- frequência do clock que o pic utiliza ; Neste caso FCLK= internal=16MHZ
    Fout? The output frequency after the division. 
    Tout ? The Cycle Time after the division. (periodo da interrupcao)
    4 - The division of the original clock (4 MHz) by 4, when using internal crystal as clock (and not external oscillator). 
    Count - A numeric value to be placed to obtain the desired output frequency - Fout. (fator necessário para ajustar Tout/Fout desejado)
    (256 - TMR0) - The number of times in the timer will count based on the register TMR0. (Ajusta até onde o timer iira contar)
     // 256 usando o timer no modo 8 bits, ou T0_16_BIT
     
     // Fout= FCLK/(4*Prescaler*(2^8-TMR0)*Count); // modo 8 bits
     // Fout= FCLK/(4*Prescaler*(2^16-TMR0)*Count); // modo 16 bits
  
     
     Para:
     TMR0=0; //valor inicial
     FCLK= internal=16MHZ
     tempo_TMR0_incremento=1/(FCLK/(4*Prescaler) ) =>  // tempo_TMR0_incremento= (Prescaler/4)us
     TMR0(max modo 16 bits)=2^16 -1 (16bits) ( 2^16 incrementos para o overflow=>TMR0=0
     
     */
    setup_timer_0(T0_INTERNAL|T0_DIV_2048| T0_16_BIT); //TMR0 incrementa a cada 512us
    
TRISA1=0; // configura o pino do led como saída

delay_us(100);

estado_led= read_eeprom(STATUS_LED_CONTROL_ADDRESS);
LATA1= estado_led; // led A1 comeca conforme seu estado do ultimo comando L usado pelo usuario

/*
 Carrega os parametros e status armazenados na EEPROM
 
 */

// Obtem o valor do  offset de calibracao da ultima calibracao realizada (salvo na eeprom)
offset_cal.valor_byte[0]= read_eeprom(OFFSET_CAL_ADDRESS);
offset_cal.valor_byte[1]= read_eeprom(OFFSET_CAL_ADDRESS +1);
offset_cal.valor_byte[2]= read_eeprom(OFFSET_CAL_ADDRESS +2);
offset_cal.valor_byte[3]= read_eeprom(OFFSET_CAL_ADDRESS +3 );

device_calibrated= read_eeprom(DEVICE_CALIBRATED_ADDRESS); // status da calibração

i2c_address=  read_eeprom(USER_I2C_ADDRESS);
if( (i2c_address>=1)&&(i2c_address<=127) ) i2c_slaveaddr(I2C_PIC_SLAVE, i2c_address << 1); // Muda o endereco I2C para o endereco da ultima vez que ele escolheu ; A condicao só serve para evitar que algum lixo na EEPROM (Geralmente ocorre pq o ICSP está apagando a EEPROM tbm)  mude o endereco da i2c (continua sendo o padrão nesse caso 98(DEC))


RESET_in_buffer ;
}


void disable_Modulos_PIC(void){

// REGISTRADORES PARA DESABILITAR\HABILITAR OS MÓDULOS DO PIC ( all modules are ON by default following any Reset.) : (CAP 14 datasheet)
/* bits :
1 = module disabled
0 = module enabled */
    
// PMD0 bits
 
// SYSCMD = 1; // 1 = System Clock network disabled (a.k.a. FOSC) See description in Section 14.3 ?System Clock Disable?.
FVRMD = 1 ; // 1 = FVR module disable
NVMMD = 1 ; //1 = Data EEPROM (a.k.a. user memory, EEPROM) reading and writing is disabled; NVMCON registers cannot be written; FSR access to EEPROM returns zero. //0 = NVM module enabled
// CLKRMD = 1 ; //  Disable Clock Reference CLKR bit
// IOCMD = 1 ;// 1 = IOC module(s) disabled ; 0 = IOC module(s) enabled ; Disable Interrupt-on-Change bit, All Ports
// Note 1: When enabling NVM, a delay of up to 1 ?s may be required before accessing data.
////////////////////////////////////////////////////////

// PMD1 bits

// NCOMD = 1;  //bit 7 NCOMD: Disable Numerically Control Oscillator bit
//TMR6MD=  1  ; //bit 6 TMR6MD: Disable Timer TMR6 bit
//TMR5MD = 1 ; //bit 5 TMR5MD: Disable Timer TMR5 bit
//TMR4MD = 1 ; //bit 4 TMR4MD: Disable Timer TMR4 bit
//TMR3MD = 1 ;  //bit 3 TMR3MD: Disable Timer TMR3 bit
//TMR2MD = 1 ; //bit 2 TMR2MD: Disable Timer TMR2 bit
//TMR1MD = 1 ; //bit 1 TMR1MD: Disable Timer TMR1 bit
TMR0MD = 1 ; //bit 0 TMR0MD: Disable Timer TMR0 bit

//bits PMD2:

DACMD = 1 ;  //bit 6 DACMD: Disable DAC bit
ADCMD = 1 ;  //bit 5 ADCMD: Disable ADC bit
CMP2MD = 1 ; //bit 2 CMP2MD: Disable Comparator C2 bit
CMP1MD = 1 ; //bit 1 CMP1MD: Disable Comparator C1 bit

/* bits PMD3:*/

CWG2MD = 1 ;    //bit 7 CWG2MD: Disable CWG2 bit
CWG1MD = 1 ;   //bit 6 CWG1MD: Disable CWG1 bit
PWM6MD = 1 ;     //bit 5 PWM6MD: Disable PWM6 bit
PWM5MD = 1 ;    //bit 4 PWM5MD: Disable PWM5 bit
CCP4MD = 1 ;     //bit 3 CCP4MD: Disable CCP4 bit
CCP3MD = 1 ;    //bit 2 CCP3MD: Disable CCP3 bit
CCP2MD = 1 ;     //bit 1 CCP2MD: Disable CCP2 bit
CCP1MD = 1 ;     //bit 0 CCP1MD: Disable CCP1 bit

/* bits PMD4:*/

//UART1MD = 1 ;   //bit 5 UART1MD: Disable EUSART1 bit
//MSSP2MD = 1 ;   //bit 2 MSSP2MD: Disable MSSP2 bit
//MSSP1MD = 1 ;   //bit 1 MSSP1MD: Disable MSSP1 bit

/* bits PMD5: */

CLC4MD = 1 ;   //bit 4 CLC4MD: Disable CLC4 bit
CLC3MD = 1 ;   //bit 3 CLC3MD: Disable CLC3 bit
CLC2MD = 1 ;    //bit 2 CLC2MD: Disable CLC2 bit
CLC1MD = 1 ;     //bit 1 CLC1MD: Disable CLC1 bit
DSMMD  = 1 ;     //bit 0 DSMMD: Disable Data Signal Modulator bit

}

void renable_Modulos_PIC(void) {

// REGISTRADORES PARA DESABILITAR\HABILITAR OS MÓDULOS DO PIC ( all modules are ON by default following any Reset.) : (CAP 14 datasheet)
/* bits :
1 = module disabled
0 = module enabled */
    
// PMD0 bits
 
// SYSCMD = 0; // 1 = System Clock network disabled (a.k.a. FOSC) See description in Section 14.3 ?System Clock Disable?.
FVRMD = 0 ; // 1 = FVR module disable
NVMMD = 0 ; //1 = Data EEPROM (a.k.a. user memory, EEPROM) reading and writing is disabled; NVMCON registers cannot be written; FSR access to EEPROM returns zero. //0 = NVM module enabled
// CLKRMD = 0 ; //  Disable Clock Reference CLKR bit
// IOCMD = 0 ;// 1 = IOC module(s) disabled ; 0 = IOC module(s) enabled ; Disable Interrupt-on-Change bit, All Ports
// Note 1: When enabling NVM, a delay of up to 1 ?s may be required before accessing data.
////////////////////////////////////////////////////////

// PMD1 bits

// NCOMD = 0;  //bit 7 NCOMD: Disable Numerically Control Oscillator bit
//TMR6MD=  0  ; //bit 6 TMR6MD: Disable Timer TMR6 bit
//TMR5MD = 0 ; //bit 5 TMR5MD: Disable Timer TMR5 bit
//TMR4MD = 0 ; //bit 4 TMR4MD: Disable Timer TMR4 bit
//TMR3MD = 0 ;  //bit 3 TMR3MD: Disable Timer TMR3 bit
//TMR2MD = 0 ; //bit 2 TMR2MD: Disable Timer TMR2 bit
//TMR1MD = 0 ; //bit 1 TMR1MD: Disable Timer TMR1 bit
TMR0MD = 0 ; //bit 0 TMR0MD: Disable Timer TMR0 bit

//bits PMD2:

DACMD = 0 ;  //bit 6 DACMD: Disable DAC bit
ADCMD = 0 ;  //bit 5 ADCMD: Disable ADC bit
//CMP2MD = 0 ; //bit 2 CMP2MD: Disable Comparator C2 bit
//CMP1MD = 0 ; //bit 1 CMP1MD: Disable Comparator C1 bit

/* bits PMD3:*/

//CWG2MD = 0 ;    //bit 7 CWG2MD: Disable CWG2 bit
//CWG1MD = 0 ;   //bit 6 CWG1MD: Disable CWG1 bit
//PWM6MD = 0 ;     //bit 5 PWM6MD: Disable PWM6 bit
//PWM5MD = 0 ;    //bit 4 PWM5MD: Disable PWM5 bit
//CCP4MD = 0 ;     //bit 3 CCP4MD: Disable CCP4 bit
//CCP3MD = 0 ;    //bit 2 CCP3MD: Disable CCP3 bit
//CCP2MD = 0 ;     //bit 1 CCP2MD: Disable CCP2 bit
//CCP1MD = 0 ;     //bit 0 CCP1MD: Disable CCP1 bit

/* bits PMD4:*/

//UART1MD = 0 ;   //bit 5 UART1MD: Disable EUSART1 bit
//MSSP2MD = 0 ;   //bit 2 MSSP2MD: Disable MSSP2 bit
//MSSP1MD = 0 ;   //bit 1 MSSP1MD: Disable MSSP1 bit

/* bits PMD5: */

//CLC4MD = 0 ;   //bit 4 CLC4MD: Disable CLC4 bit
//CLC3MD = 0 ;   //bit 3 CLC3MD: Disable CLC3 bit
//CLC2MD = 0 ;    //bit 2 CLC2MD: Disable CLC2 bit
//CLC1MD = 0 ;     //bit 1 CLC1MD: Disable CLC1 bit
//DSMMD  = 0 ;     //bit 0 DSMMD: Disable Data Signal Modulator bit

config_PIC() ; // quando os modulos são reabilitados ( após um disable)  todos os registradores associados ao modulo estão com valores do estado de reset POR, e dessa forma deve-se configurar o PIC novamente (dependendo do modulo tem que usar novamente as diretivas tbm (Ex #usei2c, caso i2c seja reabilitado))

}

int1 parsing_in_buffer(char *rcv_buffer) {
   
 unsigned int8 num_separadores=0;
 unsigned int8 index_separador_1 ;
 unsigned int8 index_separador_2 ;
 const char separador=',';
 
 for( int i=0; rcv_buffer[i]!='\0'; i++) { 
     rcv_buffer[i]=toupper(rcv_buffer[i]) ; /*coloca todo os caracteres
 de in_buffer pra maiusculo para que o usuario possa mandar comandos com caracteres minusculos e/ou maiusculos*/ 
    if(rcv_buffer[i]==separador) {
        num_separadores++;  // conta o numero de separadores do comando enviado para o pic
        // guarda o indice de onde ocorrem os separadores
        if(num_separadores==1) index_separador_1=i;
        if(num_separadores==2) index_separador_2=i;
          }     
 }
 

 switch(num_separadores){
     case 0:  // não tem separador=> CMD= in_buffer
           int i ;
           for( i=0; i<=LEN_MAX_CMD; i++) CMD[i]= rcv_buffer[i] ;    
           CMD[i]='\0';
           break;  
     case 1: // só tem um separadores => comando é da forma CMD,VALOR
            {
              int i ;
              for( i=0; rcv_buffer[i]!='\0'; i++) {
                  if(i<index_separador_1) CMD[i]= rcv_buffer[i] ;
                  if(i==index_separador_1) CMD[i]='\0';
                  if(i>index_separador_1) VALOR[i-(index_separador_1 +1)]= rcv_buffer[i] ;
                       }
              VALOR[i-(index_separador_1 +1)]= '\0' ;
              }
         break; 
     case 2: // tem dois  separadores => comando é da forma CMD,CMD2,VALOR
            {
             int i ;
             for( i=0; rcv_buffer[i]!='\0'; i++) {
                         if(i<index_separador_1) CMD[i]= rcv_buffer[i] ;
                         if(i==index_separador_1) CMD[i]='\0';
                         if((i>index_separador_1)&&(i<index_separador_2)) CMD2[i-(index_separador_1 +1)]= rcv_buffer[i];
                         if(i==index_separador_2) CMD2[i-(index_separador_1 +1)]='\0'; 
                         if(i>index_separador_2) VALOR[i-(index_separador_2 +1)]= rcv_buffer[i] ;    
                    }
             VALOR[i-(index_separador_2 +1) ]= '\0' ;
         }
         break;      
     default: // o comando não é de nenhuma forma esperada=> erro de sintaxe
         out_buffer[0]= 2; // 2 sintax error  "Excesso de separador"
         return FALSE ; // erro de sintaxe durante o parseamento (número inesperado de separadores)
         break;
 }
 
 return TRUE ; // caso o parseamento ocorra com sucesso (sem erro de sintaxe em relação a separação (não detecta se os argumentos dos comandos são corretos, somente a forma do comando))
 //#endif

}


int8 identifica_comando( char * comando) {
/*Identiicacao de qual comando foi recebido pelo PIC*/
for( int8 cmd= cmd_err; cmd<=cmd_Status ; cmd++ ) { // varre do primeiro comando ao ultimo da lista
  if ( strcmp(comando, lista_comandos[cmd] )== 0 ){ return cmd ;   }
}
 // comando invalido, já que o comando não esta na lista_de_comandos  
   return cmd_Err ;   
 }


void monta_out_buffer( int8 num_comando) {
 
  // Os comandos podem ser(ou qualquer outro acrescentado em lista_comandos[] e em enum comandos): cmd_err, cmd_Baud ,cmd_Cal, cmd_Export, cmd_Factory, cmd_Find, cmd_i, cmd_I2c, cmd_Import, cmd_L, cmd_Plock, cmd_R, cmd_Sleep, cmd_Slope,cmd_Status, cmd_T, cmd_RT
   switch(num_comando) {
     
       case cmd_Cal:
                 ANORP_CAL(); //
           break;
       case cmd_Factory:
                 ANORP_FACTORY();//
           break;
       case cmd_i:
                 ANORP_i();
           break;   
       case cmd_I2C:
               // ANORP_I2C() ;
           break;  
       case cmd_R:
                  ANORP_R();// retorna uma única leitura do valor de ph (%.2f) 
           break;   
       case cmd_Find:
                  ANORP_FIND(); //Find: LED rapidly blinks white, used to help find device
           break ; 
       case cmd_L:
                  ANORP_L(); // LED CONTROL
           break;
       case cmd_Status:
                 ANORP_STATUS(); //
           break;
       case cmd_Sleep:   
                 ANORP_SLEEP();
           break;
       case cmd_err: 
                    out_buffer[0]= 2; // 2 sintax error "Comando invalido"
           break;
   }       
      
   
}


// Implementações de cada comando
int1 isStr_float(char *str_teste)  {
    
    int8 num_ponto=0;
    int8 num_sinal_menos=0;
    
// retorna TRUE se a str for uma string float válida(número ponto flutuante positivo ou negativo Ex: 323.124, -2.32, 0.533, .32 ,-.32) ou FALSE caso contrário
    for(int i=0; str_teste[i]!='\0'; i++){

        if(i==0){ // o primeiro caracter pode ser digito(0-9) ou '-'
            if (isdigit(str_teste[0])||(str_teste[0]=='-') )  {
                 if(str_teste[0]=='-') num_sinal_menos++;
                 continue; 
            }
            else return FALSE; //caracter não válido
        }
        else{
            if ((isdigit(str_teste[i])==TRUE)||(str_teste[i]=='.')||(str_teste[i]=='-') ) {
           
                     if(str_teste[i]=='.') {num_ponto++; if(num_ponto>1) return FALSE; } // só pode ter um ponto
                     if(str_teste[i]=='-') return FALSE; //o sinal de menos só pode ocorrer se i=0
                     if(str_teste[strlen(str_teste)-1]=='.') return FALSE; // o ultimo caracter não nulo não pode ser '.'
                     if (isdigit(str_teste[i]) ) continue ; 
                  }    
            else return FALSE; //caracter não válido
        }
    }
 return TRUE;      
}

float32 get_orp_value_mV(void) {
          
            float32 mcp_value_mV = read_adc_volts_mcp3421(MCP3421_ADDRESS)*1000 ;
            return mcp_value_mV ;
}



void ANORP_R(void){
    
// resposta: 1(DEC) %.2f(ASCII) 0(DEC)
        if( (strcmp(CMD,in_buffer)==0)&&(CMD2[0]=='\0')&&(VALOR[0]=='\0')) { // comando passado é da forma R
        
        float32 orp=  get_orp_value_mV() + offset_cal.valor ; // Orp value em mV
        
        out_buffer[0]=1;
        sprintf(out_buffer+1,"%.2f",orp) ;
        }
        else out_buffer[0]= 2; // 2 sintax error
}


void ANORP_FIND(void){
    
    if( (strcmp(CMD,in_buffer)==0)&&(CMD2[0]=='\0')&&(VALOR[0]=='\0')) { // comando passado é da forma FIND
        FIND_exe= TRUE ;
        //Response: DEC NULL
        //           1  0
        out_buffer[0]=1; // response code
        in_buffer[0]=0; //sentinela do comando FIND
        
        while(in_buffer[0]==0){// fica piscando o led até o usuário enviar um caracter(in_buffer[0]=!0)
            LATA1=!LATA1 ;
            delay_ms(100);
            LATA1=!LATA1 ;
            delay_ms(100);
            restart_wdt(); // usado por causa do loop do find que pode fazer WDT estourar, resetando o pic; delay_ms reseta o WDT implicitamente se restart_wdt estiver em #use delay 
        }
        
        
        }
    
    else out_buffer[0]= 2; // 2 sintax error  
    
    FIND_exe= FALSE ;
}

void ANORP_L(void){
/* Comando sintaxe
 L,1 // LED on ; Response: 1(DEC) 0(NULL)
 L,0  //  LED off ; Response: 1(DEC) 0(NULL)
 L,? // LED state on/off? ; Response: 1(DEC) ?L,1 0(NULL) ou 1(DEC) ?L,0 0(NULL)
 */  
 if( (CMD2[0]=='\0')&&(VALOR[0]!='\0') ){
 
     if (strlen(VALOR)==1){
  
        switch(VALOR[0]){

            case '1':
                       LATA1=1 ;
                       out_buffer[0]=1; //Response: 1(DEC) 0(NULL) 
                       estado_led= TRUE;
                       write_eeprom(STATUS_LED_CONTROL_ADDRESS,estado_led);
                break;
            case '0':
                       LATA1=0 ;
                       out_buffer[0]=1; // //Response: 1(DEC) 0(NULL)  
                       estado_led= FALSE ;
                       write_eeprom(STATUS_LED_CONTROL_ADDRESS,estado_led);
                break;

            case '?':
                      // Response: 1(DEC) ?L,1 0(NULL) ou 1(DEC) ?L,0 0(NULL)
                      out_buffer[0]=1; 
                      sprintf(out_buffer+1,"L,?%d",PORTA1);
                break ;

            default:
                out_buffer[0]= 2; // 2 sintax error ; VALOR não é válido
                break;
           } 
     }
     
     else out_buffer[0]= 2; // 2 sintax error VALOR não tem tamanho 1
 }
 
 else out_buffer[0]= 2; // 2 sintax error ;
}

void ANORP_i(void){
 // Command sintax: i    // device information
// 1   ?i,ORP, 19.7
//Dec   ASCII        NULL
 out_buffer[0]=1;
 sprintf(out_buffer+1,"?i,ORP,%.2f", VERSAO_FIRMWARE);   
}


void ANORP_STATUS(void){
    
//Status voltage at Vcc pin and reason for last restart
      /*Response:   DEC         ASCII                       NULL
        //           1 ?Status,Reason_for_restart, Vcc       0
        // Reason_for_restart (Restart codes)    
            P powered off
            S software reset
            B brown out
            W watchdog
            U unknown
         */
    
if( (strcmp(CMD,in_buffer)==0)&&(CMD2[0]=='\0')&&(VALOR[0]=='\0')) { // comando passado é da forma STATUS
       
        float32 Vdd= ((10000+4700)/4700.0)*(2.048*( read_adc()/1023.0) ); // adc de 10 bits com 2.048 de referência (usando um divisor de tensao de 4K7/10K +4K7)
        
        out_buffer[0]=1; // response code
        
        switch(Reason_for_restart){
            
            case NORMAL_POWER_UP: //??? O ultimo reset foi por falta de alimentacao => ultimo reset foi quando o dispositivo "ligou" dps de ter estar sem alimentacao (Normal)      
                  sprintf(out_buffer+1,"?Status,%c,%.2f",'P',Vdd) ;
                   break;  
            case BROWNOUT_RESTART:
                  sprintf(out_buffer+1,"?Status,%c,%.2f",'B',Vdd) ;
                   break;
            case  WDT_TIMEOUT: 
                  sprintf(out_buffer+1,"?Status,%c,%.2f",'W',Vdd) ;
            case  WDT_FROM_SLEEP:
                  sprintf(out_buffer+1,"?Status,%c,%.2f",'W',Vdd) ;
                 break;  
                
            case  RESET_INSTRUCTION:
                sprintf(out_buffer+1,"?Status,%c,%.2f",'S',Vdd) ;
                break;    
            default: 
                sprintf(out_buffer+1,"?Status,%c,%.2f",'U',Vdd) ;
        } 
      
}
    else out_buffer[0]= 2; // 2 sintax error  

}


 void ANORP_SLEEP(void){
 // Sleep mode/low power : Send any character or command to awaken device 
 // Command syntax: Sleep; Resposta : no response (Do not read status byte after issuing sleep command.)
 // Consumo: 5V-  led on, 10.5 mA ; standby( 7,5 mA) sleep ( 5,8 mA) ; 
 // Consumo: 3.3 V- led on 6.65 mA  ;standby( 5.5mA) sleep (3.8 mA) ; 
 if( (strcmp(CMD,in_buffer)==0)&&(CMD2[0]=='\0')&&(VALOR[0]=='\0') ) {
    SLEEP_exe= TRUE ; // indica a execucao do modo Sleep; Usado para a interrupcao I2C não interpretar como comando quando o usuario fazer: (Send any character or command to awaken device )
    LATA1=0; // desliga o led
    disable_Modulos_PIC(); // desabilita todos os modulos do PIC para consumir menos energia (exceto alguns que eu não estou alterando ex, i2c uart, gerador do Fosc etc)
    sleep();  // comando passado é da forma Sleep
    SLEEP_exe= FALSE;
    renable_Modulos_PIC(); // reabilita e configura os modulos do Pic que estou utilizando 
 }
       

 else   out_buffer[0]= 2; // 2 sintax error 
 
  
 }

void ANORP_CAL(void) {
/*
Command syntax 
Cal,n // calibrates the ORP circuit to a set value ; Response:
Cal,clear  // delete calibration data;  Response:
Cal,? //device calibrated?;   Response:
*/

char str_clear[]= "CLEAR" ;   
char str_interrogacao[]= "?" ;

if( (CMD2[0]=='\0')&&(VALOR[0]!='\0')) // Comando é da da forma: Cal,%c

{    
    
    if(isStr_float(VALOR) ) {   // Cal,n 

    offset_cal.valor=  atof(VALOR) -get_orp_value_mV(); //
   
    // salva na eeprom
    write_eeprom(OFFSET_CAL_ADDRESS   , offset_cal.valor_byte[0] );
    write_eeprom(OFFSET_CAL_ADDRESS +1, offset_cal.valor_byte[1] );
    write_eeprom(OFFSET_CAL_ADDRESS +2, offset_cal.valor_byte[2] );
    write_eeprom(OFFSET_CAL_ADDRESS +3, offset_cal.valor_byte[3] );    
    
    device_calibrated=TRUE;
    write_eeprom(DEVICE_CALIBRATED_ADDRESS,device_calibrated);   
    
    out_buffer[0]= 1; // Response 1 NULL 
    }

    else    if(strcmp(VALOR,str_clear)==0){ // Cal,clear
                            offset_cal.valor=0;
                            
                            // salva na eeprom
                            write_eeprom(OFFSET_CAL_ADDRESS   , offset_cal.valor_byte[0] );
                            write_eeprom(OFFSET_CAL_ADDRESS +1, offset_cal.valor_byte[1] );
                            write_eeprom(OFFSET_CAL_ADDRESS +2, offset_cal.valor_byte[2] );
                            write_eeprom(OFFSET_CAL_ADDRESS +3, offset_cal.valor_byte[3] );   
                            
                            device_calibrated=FALSE;
                            write_eeprom(DEVICE_CALIBRATED_ADDRESS,device_calibrated);  
                            
                            out_buffer[0]= 1; // Response: 1 NULL             
                                        }
        
    else  if(strcmp(VALOR,str_interrogacao)==0){ // Cal,?
                                             out_buffer[0]= 1; // Response Code: 1 
                                             sprintf(out_buffer+1,"?Cal,%d",device_calibrated) ;
                                               }
                     else out_buffer[0]= 2;  // ErroSintaxe: Segundo argumento é inválido

}

else out_buffer[0]= 2;  // ErroSintaxe: comando invalido
}

void ANORP_FACTORY(void){
   
/* 
Clears calibration
LED on
Response codes enabled
 * 
 * Command syntax
 Factory  // enable factory reset
 Response: device reboot
 */
    
if( (strcmp(CMD,in_buffer)==0)&&(CMD2[0]=='\0')&&(VALOR[0]=='\0')) // Comando passada é da forma: Factory
{    
// Clears calibration
offset_cal.valor=0 ; // valor de offset de calibracao de fabrica 
write_eeprom(OFFSET_CAL_ADDRESS   , offset_cal.valor_byte[0] );
write_eeprom(OFFSET_CAL_ADDRESS +1, offset_cal.valor_byte[1] );
write_eeprom(OFFSET_CAL_ADDRESS +2, offset_cal.valor_byte[2] );
write_eeprom(OFFSET_CAL_ADDRESS +3, offset_cal.valor_byte[3] );

device_calibrated= FALSE;
write_eeprom(DEVICE_CALIBRATED_ADDRESS,device_calibrated);

//LED on
LATA1=1;
estado_led= TRUE;
write_eeprom(STATUS_LED_CONTROL_ADDRESS,estado_led);
//Response codes enabled (Falta implementar)

//Response: device reboot
out_buffer[0]=1;
//sprintf(out_buffer,"%s", "device reboot") ;

}

else out_buffer[0]= 2; // 2 sintax error  
}

void ANORP_I2C(void) {
// I2C,n // sets I2C address and reboots into I2C mode
    
if((CMD2[0]=='\0')&&(VALOR[0]!='\0')) {

// Queremos saber se n é um inteiro 
    for(int i=0; VALOR[i] != '\0' ; i++){
        if(isdigit( VALOR[i] )==FALSE)  {out_buffer[0]= 2; return;  } // 2 sintax error  (O endereco não é um inteiro)
    }
    
    i2c_address= atoi(VALOR);  //(endereco de 7 bits)
   
    // verifica se n está no range 1-127
    if( (i2c_address>=1)&&(i2c_address<=127) ) { // valor de n é válido
      
     
     
     i2c_slaveaddr(I2C_PIC_SLAVE, i2c_address << 1); //  muda o endereco i2c do PIC ; Obs: CCS usa o endereco na forma de 8 bits
     write_eeprom(USER_I2C_ADDRESS,i2c_address); // salva na EEPROM
     // reset_cpu(); // Response : device reboot
      
    }

    else out_buffer[0]= 2; // 2 sintax error  (endereco está fora do range 1-127)
    }

else out_buffer[0]= 2; // 2 sintax error (Comando escrito de maneira errada) 

}


// Funções de comunicação com o MCP3421
#ifndef debug 

//////////////////////////////////////////////////////////////////////////////////
// adc_init()
// Purpose: To initialize the MCP3421.
// Parameters: address - Optional parameter for specifying the address of the
//                       MCP3421 to initialize.  Allows for initializing multiple
//                       devices on same bus.  Driver only supports one device
//                       configuration.  Defaults to MCP3421_ADDRESS if not
//                       specified.
// Returns:    Nothing.
//////////////////////////////////////////////////////////////////////////////////

void adc_init(unsigned int8 address=MCP3421_ADDRESS)
{
  i2c_start(MCP3421_STREAM);  //send I2C start
  i2c_write(MCP3421_STREAM, MCP3421_DEVICE_CODE | (address << 1));  //send write command
  i2c_write(MCP3421_STREAM, MCP3421_MODE | MCP3421_BITS | MCP3421_GAIN);  //send device configuration
  i2c_stop(MCP3421_STREAM);  //send I2C stop
}

//////////////////////////////////////////////////////////////////////////////////
// read_adc_mcp3421()
// Purpose: To read the last adc conversion from device, raw value read from
//          device. If device configured for One-Shot mode, it will initiate the
//          conversion.  Function will wait for a new conversion before returning.
// Parameter: address - Optional parameter for specifying the address of the
//                      MCP3421 to read.  Allows for reading multiple devices on
//                      same bus.  Defaults to MCP3421_ADDRESS if not specified.
// Returns:   signed int32 or signed int16 value depending MCP3421_BITS value.
//////////////////////////////////////////////////////////////////////////////////
#if MCP3421_BITS == MCP3421_18BITS
signed int32 read_adc_mcp3421(unsigned int8 address=MCP3421_ADDRESS)
#else
signed int16 read_adc_mcp3421(unsigned int8 address=MCP3421_ADDRESS)
#endif
{
  union
  {
   #if MCP3421_BITS == MCP3421_18BITS
    signed int32 sint32;
    unsigned int8 b[4];
   #else
    signed int16 sint16;
    unsigned int8 b[2];
   #endif
  } result;
  unsigned int8 status = 0x80;

  #if MCP3421_MODE == MCP3421_ONE_SHOT
   i2c_start(MCP3421_STREAM);  //send I2C start
   i2c_write(MCP3421_STREAM, MCP3421_DEVICE_CODE | (address << 1));  //send write command
   i2c_write(MCP3421_STREAM, MCP3421_START_CONVERSTION | MCP3421_MODE | MCP3421_BITS | MCP3421_GAIN);  //initiate conversion
   i2c_stop(MCP3421_STREAM);  //send I2C stop
  #endif

   i2c_start(MCP3421_STREAM);  //send I2C start
   i2c_write(MCP3421_STREAM, MCP3421_DEVICE_CODE | (address << 1) | 1);  //send read command

  #if MCP3421_BITS == MCP3421_18BITS
   result.b[2] = i2c_read(MCP3421_STREAM, 1);  //read MSB 18 Bit mode
  #endif
   result.b[1] = i2c_read(MCP3421_STREAM, 1);  //read 2nd MSB 18 Bit mode, read MSB 16, 14 or 12 Bit mode
   result.b[0] = i2c_read(MCP3421_STREAM, 1);  //read LSB
   status = i2c_read(MCP3421_STREAM, 1);       //read Status

   if(bit_test(status,7))  //if RDY = 1, New conversion not ready
   {
     do
     {
       status = i2c_read(MCP3421_STREAM, 1);  //read Status
     } while(bit_test(status, 7)); //until RDY = 0

     status = i2c_read(MCP3421_STREAM, 0);  //read Status, do nack
     i2c_stop();  //send I2C stop

     i2c_start(MCP3421_STREAM);  //send I2C start
     i2c_write(MCP3421_STREAM, MCP3421_DEVICE_CODE | (address << 1) | 1);  //send read command

    #if MCP3421_BITS == MCP3421_18BITS
     result.b[2] = i2c_read(MCP3421_STREAM, 1);  //read MSB 18 Bit mode
    #endif
     result.b[1] = i2c_read(MCP3421_STREAM, 1);  //read 2nd MSB 18 Bit mode, read MSB 16, 14 or 12 Bit mode
     result.b[0] = i2c_read(MCP3421_STREAM, 1);  //read LSB
   }

   status = i2c_read(MCP3421_STREAM, 0);  //read Status, do nack
   i2c_stop();  //send I2C stop

  #if MCP3421_BITS == MCP3421_18BITS
   if(bit_test(result.b[2],1))  //if 18 Bit mode check sign bit
     result.b[3] = 0xFF;
   else
     result.b[3] = 0;
  #endif

  #if MCP3421_BITS == MCP3421_18BITS
   return(result.sint32);
  #else
   return(result.sint16);
  #endif
}

//////////////////////////////////////////////////////////////////////////////////
// read_adc_volts_mcp3421()
// Purpose: To read the last adc conversion from device, actual volt value read
//          from device.
// Parameter: address - Optional parameter for specifying the address of the
//                      MCP3421 to read.  Allows for reading multiple devices on
//                      same bus.  Defaults to MCP3421_ADDRESS if not specified.
// Returns:   float32
//////////////////////////////////////////////////////////////////////////////////
float32 read_adc_volts_mcp3421(unsigned int8 address=MCP3421_ADDRESS)
{
  #if MCP3421_BITS == MCP3421_18BITS
   signed int32 result;
  #else
   signed int16 result;
  #endif

  float32 fresult;

  result = read_adc_mcp3421(address);

  #if MCP3421_BITS == MCP3421_12BITS
   fresult = (float32)result * 0.001;
  #elif MCP3421_BITS == MCP3421_14BITS
   fresult = (float32)result * 0.00025;
  #elif MCP3421_BITS == MCP3421_16BITS
   fresult = (float32)result * 0.0000625;
  #else
   fresult = (float32)result * 0.000015625;
  #endif

  #if MCP3421_GAIN == MCP3421_8X_GAIN
   fresult /= 8;
  #elif MCP3421_GAIN == MCP3421_4X_GAIN
   fresult /= 4;
  #elif MCP3421_gain == MCP3421_2X_GAIN
   fresult /= 2;
  #endif

   return(fresult);
}

#endif
