#include <unistd.h>
#include <string.h>
#include "py/mpconfig.h"
#include "../condStates.h"

extern char buff_micropython[10240];

/*
 * Core UART functions to implement for a port
 */

#if MICROPY_MIN_USE_STM32_MCU
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
} periph_uart_t;
#define USART1 ((periph_uart_t*)0x40011000)
#endif

// Receive single character
int mp_hal_stdin_rx_chr(void) {
    unsigned char c = 0;
#if MICROPY_MIN_USE_STDOUT
    int r = read(0, &c, 1);
    (void)r;
#elif MICROPY_MIN_USE_STM32_MCU
    // wait for RXNE
    while ((USART1->SR & (1 << 5)) == 0) {
    }
    c = USART1->DR;
#endif
    return c;
}

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    strncat(buff_micropython,str,len);
    if(buff_micropython[strlen(buff_micropython)-1]=='\n'){
        pthread_mutex_lock(&mutex);
        state = SEND_OUTPUT;
        pthread_cond_signal(&pulgaTurn);
        pthread_mutex_unlock(&mutex);
        // Waits until the output is sent
        pthread_mutex_lock(&mutex);
        while (state != EXECUTE_PYTHON)
            pthread_cond_wait(&pythonTurn, &mutex);
        pthread_mutex_unlock(&mutex);
    }
    
    
#if MICROPY_MIN_USE_STDOUT
    int r = write(1, str, len);
    (void)r;
#elif MICROPY_MIN_USE_STM32_MCU
    while (len--) {
        // wait for TXE
        while ((USART1->SR & (1 << 7)) == 0) {
        }
        USART1->DR = *str++;
    }
#endif
}
