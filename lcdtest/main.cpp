#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include <stdlib.h>
#include <math.h>

#include "test.h"

#define LCM_RESET 1
#define LCM_SPI_CLK 2
#define LCM_SPI_MOSI 3
#define LCM_DC 0

#define LCM_WIDTH 128
#define LCM_HEIGHT 160

int bulkDMAChannel;
dma_channel_config cfg;
unsigned char* backBuffer;

void lcmCommand(unsigned char byte)
{
    gpio_put(LCM_DC, 0);
    spi_write_blocking(spi0, &byte, sizeof(byte));
}

void lcmData(unsigned char byte)
{
    gpio_put(LCM_DC, 1);
    spi_write_blocking(spi0, &byte, sizeof(byte));
}

void lcmFlush()
{
    gpio_put(LCM_DC, 1);
    spi_write_blocking(spi0, backBuffer, 128 * 160 * 2);
}

void lcmInitInterface()
{
	printf("LCM: Initializing SPI interface...\n");

    gpio_set_function(LCM_SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(LCM_SPI_MOSI, GPIO_FUNC_SPI);

	// HW reset
	gpio_init(LCM_RESET);
    gpio_set_dir(LCM_RESET, true);
    gpio_put(LCM_RESET, false);
    sleep_ms(400);
    gpio_put(LCM_RESET, true);

    gpio_init(LCM_DC);
    gpio_set_dir(LCM_DC, true);

    spi_init(spi0, 105535000);
}

void lcmAllocBackBuffer()
{
	printf("LCM: Allocating %ix%i backbuffer of RGB565 format\n", LCM_WIDTH, LCM_HEIGHT);
	int backBufSize = LCM_WIDTH * LCM_HEIGHT * 2 + 1;
	backBuffer = (unsigned char*)malloc(backBufSize);
	
	printf("LCM: Setting up DMA channel...\n");
	bulkDMAChannel = dma_claim_unused_channel(true);
	cfg = dma_channel_get_default_config(bulkDMAChannel);
	channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
	channel_config_set_dreq(&cfg, spi_get_dreq(spi0, true));
}

void st7735Init()
{
    lcmCommand(0x11);//Sleep out
	sleep_ms(120);
	//ST7735R Frame Rate
	lcmCommand(0xB1);
	lcmData(0x01);
	lcmData(0x2C);
	lcmData(0x2D);
	lcmCommand(0xB2);
	lcmData(0x01);
	lcmData(0x2C);
	lcmData(0x2D);
	lcmCommand(0xB3);
	lcmData(0x01);
	lcmData(0x2C);
	lcmData(0x2D);
	lcmData(0x01);
	lcmData(0x2C);
	lcmData(0x2D);
	//------------------------------------End ST7735R Frame Rate-----------------------------------------//
	lcmCommand(0xB4);//Column inversion
	lcmData(0x07);
	//------------------------------------ST7735R Power Sequence-----------------------------------------//
	lcmCommand(0xC0);
	lcmData(0xA2);
	lcmData(0x02);
	lcmData(0x84);
	lcmCommand(0xC1);
	lcmData(0xC5);
	lcmCommand(0xC2);
	lcmData(0x0A);
	lcmData(0x00);
	lcmCommand(0xC3);
	lcmData(0x8A);
	lcmData(0x2A);
	lcmCommand(0xC4);
	lcmData(0x8A);
	lcmData(0xEE);
	//---------------------------------End ST7735R Power Sequence-------------------------------------//
	lcmCommand(0xC5);//VCOM
	lcmData(0x0E);
	lcmCommand(0x36);//MX, MY, RGB mode
	lcmData(0xC8);
	//------------------------------------ST7735R Gamma Sequence-----------------------------------------//
	lcmCommand(0xe0);
	lcmData(0x02);
	lcmData(0x1c);
	lcmData(0x07);
	lcmData(0x12);
	lcmData(0x37);
	lcmData(0x32);
	lcmData(0x29);
	lcmData(0x2d);
	lcmData(0x29);
	lcmData(0x25);
	lcmData(0x2b);
	lcmData(0x39);
	lcmData(0x00);
	lcmData(0x01);
	lcmData(0x03);
	lcmData(0x10);
	lcmCommand(0xe1);
	lcmData(0x03);
	lcmData(0x1d);
	lcmData(0x07);
	lcmData(0x06);
	lcmData(0x2e);
	lcmData(0x2c);
	lcmData(0x29);
	lcmData(0x2d);
	lcmData(0x2e);
	lcmData(0x2e);
	lcmData(0x37);
	lcmData(0x3f);
	lcmData(0x00);
	lcmData(0x00);
	lcmData(0x02);
	lcmData(0x10);
	lcmCommand(0x2A);
	lcmData(0x00);
	lcmData(0x02);
	lcmData(0x00);
	lcmData(0x81);

	lcmCommand(0x2B);
	lcmData(0x00);
	lcmData(0x01);
	lcmData(0x00);
	lcmData(0xA0);
	//------------------------------------End ST7735R Gamma Sequence-----------------------------------------//

    //lcmCommand(0x3A);
    //lcmData(0x05);
	lcmCommand(0x3A);//65k mode
	lcmData(0x05);
    lcmCommand(0x2C);//Display on
    
    lcmCommand(0x29);//Display on

	// Set viewport
    lcmCommand(0x2A);
    lcmData(0 >> 8);
    lcmData(0 & 0xFF);
    lcmData(128 >> 8);
    lcmData(128 & 0xFF);

    lcmCommand(0x2B);
    lcmData(0 >> 8);
    lcmData(0 & 0xFF);
    lcmData(160 >> 8);
    lcmData(160 & 0xFF);
}

void lcmInit()
{
	printf("LCM: Initializing ST7735 controller\n");
	st7735Init();

	lcmCommand(0x2A);
    lcmData(0 >> 8);
    lcmData(0 & 0xFF);
    lcmData(128 >> 8);
    lcmData(128 & 0xFF);

    lcmCommand(0x2B);
    lcmData(0 >> 8);
    lcmData(0 & 0xFF);
    lcmData(160 >> 8);
    lcmData(160 & 0xFF);
	lcmCommand(0x2C);
}

/* Graphics routines */

__inline void pixelAt(short x, short y, short color)
{
    if(x < 0 || y < 0 || x >= LCM_WIDTH || y >= LCM_HEIGHT)
        return;

    unsigned char* col = (unsigned char*)&color;
    *((short*)&backBuffer[(y * 128 + x) * 2]) = color;
}

void grDrawBitmapTransparent(void* bmp, int width, int height, int x, int y)
{
    short* pixels = (short*)bmp;

	if(x > LCM_WIDTH)
		return;

	if(y > LCM_HEIGHT)
		return;

	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			if(y + j >= LCM_HEIGHT)
				break;
			
			short pixel = pixels[j * width +i];

            pixelAt(x + i, y + j, pixel);
		}

		if(x + i >= LCM_WIDTH)
			break;
	}
}

int main() {
    stdio_init_all();
    sleep_ms(1000);

    printf("LCM test by monobogdan\n");
    
    lcmInitInterface();
    lcmAllocBackBuffer();
    lcmInit();

    grDrawBitmapTransparent((void*)&habr, HABR_WIDTH, HABR_HEIGHT, 0, 0);
    lcmFlush();

    while(1)
    {
        
    }
}
