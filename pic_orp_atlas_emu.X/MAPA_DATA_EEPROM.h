/* 
 * File:   mapa_eeprom.h
 * Author: Leonardo
 *
 * Created on 10 de Julho de 2019, 10:57
 */

#ifndef MAPA_EEPROM_H
#define	MAPA_EEPROM_H

#define OFFSET_CAL_ADDRESS 10  // endereco de offset_cal na eeprom
#define DEVICE_CALIBRATED_ADDRESS 0 // endereco do status de calibracao
#define STATUS_LED_CONTROL_ADDRESS 1 // endereco do status do led (usado no comando L)
#define BAUD_RATE_ADDRESS 2 // armazena o valor do BAUD_RATE do MODO UART
#define USER_I2C_ADDRESS 3// endereco i2c do PIC escolhido pelo usuario

/*MAPA E2PROM
 * config_fabrica       0   1 byte      uint8
 * operation            1   1 byte      uint8      
 * i2c_address          2   1 byte      uint8

 * device_calibrated    DEVICE_CALIBRATED_ADDRESS   1 byte         unsigned int8
 * baud_rate            BAUD_RATE_ADDRESS           2 bytes        unsigned int16
 * offset_cal           DEVICE_CALIBRATED_ADDRESS   4 bytes    float32 e unsigned int8 (union)
 * estado_led           STATUS_LED_CONTROL_ADDRESS  1 byte         unsigned int8
 */





#endif	/* MAPA_EEPROM_H */

