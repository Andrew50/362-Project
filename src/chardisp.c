#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "chardisp.h"

extern const int SPI_DISP_SCK; extern const int SPI_DISP_CSn; extern const int SPI_DISP_TX;
extern const bool USING_LCD;

/***************************************************************** */

void init_chardisp_pins() {
    spi_inst_t *spi = spi0;
    
    gpio_set_function(SPI_DISP_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_TX, GPIO_FUNC_SPI);
    gpio_set_function(SPI_DISP_CSn, GPIO_FUNC_SPI);
    
    spi_init(spi, 10000);
    
    if (USING_LCD) {
        spi_set_format(spi, 9, 0, 0, SPI_MSB_FIRST);
    } else {
        spi_set_format(spi, 10, 0, 0, SPI_MSB_FIRST);
    }
}

void send_spi_cmd(spi_inst_t* spi, uint16_t value) {
    while (spi_is_busy(spi)) {
        tight_loop_contents();
    }
    
    spi_write16_blocking(spi, &value, 1);
}

void send_spi_data(spi_inst_t* spi, uint16_t value) {
    if (USING_LCD) {
        value |= 0x100;
    } else {
        value |= 0x200;
    }
    
    send_spi_cmd(spi, value);
}

void cd_init() {
    spi_inst_t *spi = spi0;
    
    if (USING_LCD) {
        sleep_ms(1);
        
        send_spi_cmd(spi, 0x3C);
        sleep_us(40);
        
        send_spi_cmd(spi, 0x0C);
        sleep_us(40);
        
        send_spi_cmd(spi, 0x01);
        sleep_ms(2);
        
        send_spi_cmd(spi, 0x06);
        sleep_us(40);
        
    } else {
        sleep_ms(1);
        
        send_spi_cmd(spi, 0x38);
        
        send_spi_cmd(spi, 0x0C);
        
        send_spi_cmd(spi, 0x01);
        sleep_ms(2);
        
        send_spi_cmd(spi, 0x06);
        
        send_spi_cmd(spi, 0x02);
    }
}

void cd_display1(const char *str) {
    spi_inst_t *spi = spi0;
    
    send_spi_cmd(spi, 0x80);
    
    for (int i = 0; i < 16 && str[i] != '\0'; i++) {
        send_spi_data(spi, str[i]);
    }
}

void cd_display2(const char *str) {
    spi_inst_t *spi = spi0;
    
    send_spi_cmd(spi, 0xC0);
    
    for (int i = 0; i < 16 && str[i] != '\0'; i++) {
        send_spi_data(spi, str[i]);
    }
}

/***************************************************************** */