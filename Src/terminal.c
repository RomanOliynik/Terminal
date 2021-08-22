
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "stm32g0xx_hal.h"
#include "errorHandlers.h"
#include "terminal.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "main.h"



    
/*______________________________
- - - - - - - - - - - - - - - - - 
______________________________*/
void terminal_init(void)
    {
        MX_GPIO_Init();
        MX_DMA_Init();
        MX_ADC1_Init();
        MX_DAC1_Init();
        MX_TIM1_Init();
        MX_USART2_UART_Init();
        MX_TIM3_Init();
        MX_I2C1_Init();
    }
    
/*______________________________
- - - - - - - - - - - - - - - - - 
______________________________*/
int Char_ToGPIOx(GPIO_TypeDef **gpio_tport, char *port_str)
{
    uint8_t port;
       if(strlen(port_str) == 1) {
        port = port_str[0];
    }
    else {
        return EXIT_FAILURE;
    }                 
    if(islower(port)) {
        port = toupper(port);
    }
                
    switch(port) {
        case 'A': *gpio_tport = GPIOA;   break;
        case 'B': *gpio_tport = GPIOB;   break;
        case 'C': *gpio_tport = GPIOC;   break;        
        case 'D': *gpio_tport = GPIOD;   break;
        case 'F': *gpio_tport = GPIOF;   break;
        default: return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;   
}


/*______________________________
- - - - - - - - - - - - - - - - - 
______________________________*/
int Str_ToPin(char *gpio_str, uint16_t *gpio_hwpin) {
    
    uint16_t gpio_pin;
     
    gpio_pin |= (1 << (gpio_str[0] - 0x30));
    *gpio_hwpin = gpio_pin;
    return EXIT_SUCCESS;       
}

int execfunc(int quantity, char *arguments[])
{
    GPIO_TypeDef *gpio_port;
  uint16_t gpio_pin;
  
  
  if(!strcmp(arguments[0], "pin_set"))// set pin in state "1"
  {
    if(quantity == 3)
    {
      if((Char_ToGPIOx(&gpio_port, arguments[1]) == EXIT_SUCCESS) && (Str_ToPin(arguments[2], &gpio_pin) == EXIT_SUCCESS))
      {
        gpio_port -> BSRR = (uint32_t) gpio_pin;
        /* or we can write like that*/
         //gpio_port -> ODR |= (uint16_t) gpio_pin;
        return EXIT_SUCCESS;
      }
    }
  }
  
  if(!strcmp(arguments[0], "pin_reset"))  // set pin in state "0"
  {
    if(quantity == 3)
    {
      if((Char_ToGPIOx(&gpio_port, arguments[1]) == EXIT_SUCCESS) && (Str_ToPin(arguments[2], &gpio_pin) == EXIT_SUCCESS))
      {
        gpio_port -> BRR = (uint32_t) gpio_pin;
        /* or we can write like that*/
         //gpio_port -> ODR &= ~(uint16_t) gpio_pin;
        return EXIT_SUCCESS;
      }
    }
  }
    
    if(!strcmp(arguments[0], "adc_start")) // starts ADC with number of conversations
    {
        if(quantity == 2)
        {        
            int k = atoi(arguments[1]); // tells how much conversions to make
            char str[25]; // for writing result on screen
            uint32_t adcValue[k]; // for saving data from ADC 
            int m = 0;
            ADC_Start_DMA(adcValue, k); // start conversation of ADC throw dma
            HAL_Delay(100);
            while(1)
            {
                if ( m < k)
                    {
                    ADC_Start(); // start ADC
                    sprintf(str, "%0d received data is: %d \n\r", m+1, adcValue[m]);
                    UART_Transmit( (uint8_t *) str, 27, 100); // transmition of the result
                    HAL_Delay(100);
                    m++;
                    }
                    else break;
                }
            return EXIT_SUCCESS;
        }
    }
    
    
    if(!strcmp(arguments[0], "dac_start")) // starts DAC with number to convert
    {
        if (quantity == 2)
        {
            DAC_Start();
            uint16_t data = atoi(arguments[1]);
            DAC_SetValue(data);
            DAC_Stop();
        }
        return EXIT_SUCCESS;
    }
    
    if(!strcmp(arguments[0], "pwm")) // starts PWM generation with some dutyCycle 
    {
        if (quantity == 2)
        {
            
            TIM_PWM_Start();// start PWM
            uint32_t duty = atoi(arguments[1]);
            htim1.Instance ->CCR1 = duty; // rewriting duty cycle from command to command register of timer
        }
        return EXIT_SUCCESS;
    }
    
    if (!strcmp(arguments[0], "timer")) //starts to count down from some number
    {
        if(quantity == 2)
        {
            timerTime = atoi(arguments[1]);
            Timer_Start();
            
            
        }
        return EXIT_SUCCESS;
    }
    
    if(!strcmp(arguments[0], "mem")) // can connect with external memory
    {
        if (quantity == 5)
            {
                if(!strcmp(arguments[1], "write"))
                {
                    int k = strlen(arguments[4]);
                    uint16_t devAdr =  atoi(arguments[2]);
                    uint16_t memAdr =  atoi(arguments[3]);
                    uint8_t data_to_write[k];
                    char str[k+26];
                    sprintf((char *)data_to_write, arguments[4]);
                    //memcpy(data_to_write, arguments[4], sizeof(arguments[4]));
                    MemoryWrite(devAdr, memAdr, data_to_write, k);
                    HAL_Delay(100);
                    sprintf(str, "You wrote to the memory %s\n\r", data_to_write);
                    UART_Transmit((uint8_t *)str, strlen(str), HAL_MAX_DELAY);
                    UART_Transmit("Writing is ended!\n\r", 20, HAL_MAX_DELAY);
                    return EXIT_SUCCESS;
                }
                else if(!strcmp(arguments[1], "read"))
                {
                    uint16_t devAdr =  atoi(arguments[2]);
                    uint16_t memAdr =  atoi(arguments[3]);
                    int k = atoi(arguments[4]);
                    uint8_t data_to_read[k];
                    uint8_t transmit[k];
                    MemoryRead(devAdr, memAdr, data_to_read, k);
                    char str[59];
                    sprintf(str, "You want to read %d bytes. This is the result of reading:\n\r",k);
                    memcpy(transmit, data_to_read, sizeof(data_to_read));
                    UART_Transmit((uint8_t *)str, 59, HAL_MAX_DELAY);
                    UART_Transmit(transmit, sizeof(transmit), HAL_MAX_DELAY);
                    UART_Transmit("\n\r", 2, 10);
                    //UART_Transmit((uint8_t *)data_to_read, sizeof(data_to_read),HAL_MAX_DELAY);
                //    UART_Transmit("\n\r", 2, 10);
                    return EXIT_SUCCESS;
                }
            }
    }
  return EXIT_FAILURE;
}

/*______________________________
- - - - - - - - - - - - - - - - - 
______________________________*/
void terminalParse(char* command, int command_size)
{
    char *command_buff = command;
  int str_size = command_size;
  
             command_buff[str_size - 1] = '\0'; // deleting and changing the last symbol with 0                       
             char *argument[10];
             char *piece = strtok (command_buff," \r\n"); // divide into lexems
                  
             int i = 0;
             while (piece != NULL)  
                {
                  argument[i++] = piece;  // counting how much lexems we receive
                  piece = strtok (NULL, " \r\n"); 
                }
             if(command_buff[0] != 0) 
                {
                  execfunc(i, argument);  // try execute command
                 }
}
