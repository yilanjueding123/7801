/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include "msgevt.h"

#include "drv_l1_spi.h"
#include "drv_l1_gpio.h"
#include "drv_l1_ext_int.h"

#include "ssv_drv.h"
#include "ssv_drv_config.h"
#include "spi_def.h"
#include "spi.h"

#if (HOST_PLATFORM_SEL == GP_19B)
 #define SPI_SYSCLK_4             SYSCLK_4
 #define SPI_SYSCLK_8             SYSCLK_8
 #define SSV_SPI_NUM              SPI_1
 #define SSV_CS_PIN               IO_A15
 #define SSV_IRQ_PIN              IO_F5
 #define GP_SPI_TRANSCEIVE_CPU    spi_transceive_cpu
 #define GP_SPI_TRANSCEIVE        spi_transceive
 #define GP_SPI_CLK_SET           spi_clk_set

#elif (HOST_PLATFORM_SEL == GP_15B)
 #define SPI_SYSCLK_4             24000000
 #define SPI_SYSCLK_8             12000000
 #define SSV_SPI_NUM              SPI_0
 #define SSV_CS_PIN               IO_D10
 #define SSV_IRQ_PIN              IO_C13//IO_B9
 #define GP_SPI_TRANSCEIVE_CPU    drv_l1_spi_transceive_cpu
 #define GP_SPI_TRANSCEIVE        drv_l1_spi_transceive
 #define GP_SPI_CLK_SET           drv_l1_spi_clk_set

#else
 # error "Error GP Platform selection!!"
#endif

u8 temp_wbuf[1600]={0};

static void _spi_host_cs_init(void);
static void _spi_host_irq_pin_init(void (*spi_isr)(void));
static void _spi_host_cs_high(void);
static void _spi_host_cs_low(void);

static void _spi_host_cs_init(void)
{

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
#if (HOST_PLATFORM_SEL == GP_19B)
    gpio_set_memcs(1,0); //cs1 set as IO
#elif (HOST_PLATFORM_SEL == GP_15B)
    R_GPIOCTRL |= (0x01<<3); // CS Pin[IOD6] change to GPIO Function   
#endif
    gpio_init_io(SSV_CS_PIN,GPIO_OUTPUT);
    gpio_set_port_attribute(SSV_CS_PIN, 1);
    gpio_write_io(SSV_CS_PIN, 1);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
}



static void _spi_host_irq_pin_init(void (*spi_isr)(void))
{

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    gpio_init_io(SSV_IRQ_PIN,GPIO_INPUT);
    gpio_set_port_attribute(SSV_IRQ_PIN, 0);
    gpio_write_io(SSV_IRQ_PIN, 0);

#if (HOST_PLATFORM_SEL == GP_19B)
    extab_edge_set(EXTA,RISING); /* rising trigger */
    extab_user_isr_set(EXTA,spi_isr); /* register ext A interrupt callback function */
    extab_enable_set(EXTA,FALSE);
#elif (HOST_PLATFORM_SEL == GP_15B)
    drv_l1_ext_int_init();
    drv_l1_extabc_edge_set(EXTA,RISING); /* rising trigger */
    drv_l1_extabc_user_isr_set(EXTA,spi_isr); /* register ext A interrupt callback function */
    drv_l1_extabc_enable_set(EXTA,FALSE);
#endif
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
}


static void _spi_host_cs_high(void)
{
    gpio_write_io(SSV_CS_PIN, 1);
}

static void _spi_host_cs_low(void)
{
    gpio_write_io(SSV_CS_PIN, 0);
}
void (*host_isr)(void);
void platform_spi_isr(void)
{
    if(host_isr)
        host_isr();
}

bool is_truly_isr()
{
    return TRUE;
}

bool spi_host_init(void (*spi_isr)(void))
{
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    if(spi_isr==NULL)
    {
        SDRV_ERROR("%s(): spi_isr is a null pointer\r\n",__FUNCTION__);
        return FALSE;
    }
#if (HOST_PLATFORM_SEL == GP_19B)
    spi_init(SSV_SPI_NUM);
    spi_cs_by_external_set(SSV_SPI_NUM);
    spi_clk_set(SSV_SPI_NUM, SYSCLK_4);
#elif (HOST_PLATFORM_SEL == GP_15B)
    drv_l1_spi_init(SSV_SPI_NUM);
    drv_l1_spi_cs_by_gpio_set(SSV_SPI_NUM,0,SSV_CS_PIN,1);
    drv_l1_spi_clk_set(SSV_SPI_NUM, 24000000);
#endif
    _spi_host_cs_init();
    _spi_host_cs_high();    
    host_isr = spi_isr;
    _spi_host_irq_pin_init(platform_spi_isr);
    OS_MemSET(temp_wbuf,0,sizeof(temp_wbuf));
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return TRUE;
}

bool spi_host_readwrite(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u8 options, bool IsRead)
{
    bool ret=TRUE;
    if(options & SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE)
    {
        _spi_host_cs_low();
    }
    if(options & SPI_TRANSFER_OPTIONS_CPU_POLLING_MODE)
    {
        GP_SPI_CLK_SET(SSV_SPI_NUM, SPI_SYSCLK_8);
        if(TRUE==IsRead){
            //OS_MemSET(buf,0,sizeToTransfer);
            if(STATUS_OK != GP_SPI_TRANSCEIVE_CPU(SSV_SPI_NUM, temp_wbuf, sizeToTransfer, buf, sizeToTransfer)) ret=FALSE;
        }else{
            if(STATUS_OK != GP_SPI_TRANSCEIVE_CPU(SSV_SPI_NUM, buf, sizeToTransfer, buf, sizeToTransfer)) ret=FALSE;
        }
        GP_SPI_CLK_SET(SSV_SPI_NUM, SPI_SYSCLK_4);
    }
    else
    {

        if(TRUE==IsRead){
            //OS_MemSET(buf,0,sizeToTransfer);
            if(STATUS_OK != GP_SPI_TRANSCEIVE(SSV_SPI_NUM, temp_wbuf, sizeToTransfer, buf, sizeToTransfer)) ret=FALSE;
        }else{
            if(STATUS_OK != GP_SPI_TRANSCEIVE(SSV_SPI_NUM, buf, sizeToTransfer, buf, 0)) ret=FALSE;
        }
        //if(STATUS_OK != spi_transceive(SSV_SPI_NUM, buf, sizeToTransfer, buf, (IsRead==TRUE)?sizeToTransfer:0)) ret=FALSE;
    }
    if(options & SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE)
    {
        _spi_host_cs_high();
    }
    return ret;
}
#if 0
bool spi_host_write(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options)
{
    return spi_host_readwrite(buf, sizeToTransfer, sizeToTransfered, options, FALSE);
}

bool spi_host_read(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options)
{
    return spi_host_readwrite(buf, sizeToTransfer, sizeToTransfered, options, TRUE);
}
#endif
void spi_host_irq_enable(bool enable)
{
#if (HOST_PLATFORM_SEL == GP_19B)
    extab_enable_set(EXTA,enable);
#elif (HOST_PLATFORM_SEL == GP_15B)
    drv_l1_extabc_enable_set(EXTA,enable);
#endif
}

bool spi_host_detect_card(void)
{
    return TRUE;
}


