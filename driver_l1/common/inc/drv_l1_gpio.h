#ifndef __drv_l1_GPIO_H__
#define __drv_l1_GPIO_H__
 
#include "driver_l1.h"
#include "drv_l1_sfr.h"
#include "drv_l1_tools.h"

/*GPIO Register define*/
/* GPIO: 0xC0000000 */
#define GPIO_BASE_ADDR             	0xC0000000
#define EACH_GPIO_DATA_REG_OFFSET   0x00000020 
#define EACH_DIR_REG_OFFSET        	0x00000020
#define EACH_ATTRIB_REG_OFFSET     	0x00000020

#define IOA_DATA_ADDR              		(GPIO_BASE_ADDR+0x00)   /*0xC0000000*/ 
#define IOA_BUFFER_ADDR            		(GPIO_BASE_ADDR+0x04)   /*0xC0000004*/ 
#define IOA_DIR_ADDR               		(GPIO_BASE_ADDR+0x08)   /*0xC0000008*/ 
#define IOA_ATTRIB_ADDR            		(GPIO_BASE_ADDR+0x0C)   /*0xC000000C*/ 
#define IOA_DRV	                        (GPIO_BASE_ADDR+0x10)   /*0xC0000010*/ 
                                                                               
#define IOB_DATA_ADDR              		(GPIO_BASE_ADDR+0x20)   /*0xC0000020*/ 
#define IOB_BUFFER_ADDR            		(GPIO_BASE_ADDR+0x24)   /*0xC0000024*/ 
#define IOB_DIR_ADDR               		(GPIO_BASE_ADDR+0x28)   /*0xC0000028*/ 
#define IOB_ATTRIB_ADDR            		(GPIO_BASE_ADDR+0x2C)   /*0xC000002C*/ 
#define IOB_DRV	                        (GPIO_BASE_ADDR+0x30)   /*0xC0000030*/ 
                                                                               
#define IOC_DATA_ADDR              		(GPIO_BASE_ADDR+0x40)   /*0xC0000040*/ 
#define IOC_BUFFER_ADDR            		(GPIO_BASE_ADDR+0x44)   /*0xC0000044*/ 
#define IOC_DIR_ADDR              	 	(GPIO_BASE_ADDR+0x48)   /*0xC0000048*/ 
#define IOC_ATTRIB_ADDR            		(GPIO_BASE_ADDR+0x4C)   /*0xC000004C*/ 
#define IOC_DRV	                        (GPIO_BASE_ADDR+0x50)   /*0xC0000050*/ 
                                                                               
#define IOD_DATA_ADDR              		(GPIO_BASE_ADDR+0x60)   /*0xC0000060*/ 
#define IOD_BUFFER_ADDR            		(GPIO_BASE_ADDR+0x64)   /*0xC0000064*/ 
#define IOD_DIR_ADDR               		(GPIO_BASE_ADDR+0x68)   /*0xC0000068*/ 
#define IOD_ATTRIB_ADDR            		(GPIO_BASE_ADDR+0x6C)   /*0xC000006C*/ 
#define IOD_DRV	                        (GPIO_BASE_ADDR+0x70)   /*0xC0000070*/ 
                                                                               
#define IOE_DATA_ADDR              		(GPIO_BASE_ADDR+0x80)   /*0xC0000080*/ 
#define IOE_BUFFER_ADDR            		(GPIO_BASE_ADDR+0x84)   /*0xC0000084*/ 
#define IOE_DIR_ADDR               		(GPIO_BASE_ADDR+0x88)   /*0xC0000088*/ 
#define IOE_ATTRIB_ADDR            		(GPIO_BASE_ADDR+0x8C)   /*0xC000008C*/ 
#define IOE_DRV	                        (GPIO_BASE_ADDR+0x90)   /*0xC0000090*/ 


/* Attribution Register High/Low definition */
#define INPUT_NO_RESISTOR       1
#define OUTPUT_UNINVERT_CONTENT 1 
#define INPUT_WITH_RESISTOR     0
#define OUTPUT_INVERT_CONTENT   0


#define GPIO_FAIL               0
#define GPIO_OK                 1

#define EACH_REGISTER_GPIO_NUMS 16

#define LOWEST_BIT_MASK         0x00000001


#define DRV_WriteReg32(addr,data)     ((*(volatile INT32U *)(addr)) = (INT32U)data)
#define DRV_Reg32(addr)               (*(volatile INT32U *)(addr))



/* Bearer type enum */
typedef enum 
{
	GPIO_SET_A=0,
	GPIO_SET_B,
	GPIO_SET_C,
	GPIO_SET_D,
    GPIO_SET_E,
    GPIO_SET_MAX
} GPIO_SET_ENUM;


#ifndef __GPIO_TYPEDEF__
#define __GPIO_TYPEDEF__

typedef enum {
    IO_A0=0,
    IO_A1 ,
    IO_A2 ,
    IO_A3 ,
    IO_A4 ,
    IO_A5 ,
    IO_A6 ,
    IO_A7 ,
    IO_A8 ,
    IO_A9 ,
    IO_A10,
    IO_A11,
    IO_A12,
    IO_A13,
    IO_A14,
    IO_A15,
    IO_B0 ,
    IO_B1 ,
    IO_B2 ,
    IO_B3 ,
    IO_B4 ,
    IO_B5 ,
    IO_B6 ,
    IO_B7 ,
    IO_B8 ,
    IO_B9 ,
    IO_B10,
    IO_B11,
    IO_B12,
    IO_B13,
    IO_B14,
    IO_B15,
    IO_C0 ,
    IO_C1 ,
    IO_C2 ,
    IO_C3 ,
    IO_C4 ,
    IO_C5 ,
    IO_C6 ,
    IO_C7 ,
    IO_C8 ,
    IO_C9 ,
    IO_C10,
    IO_C11,
    IO_C12,
    IO_C13,
    IO_C14,
    IO_C15,
    IO_D0 ,
    IO_D1 ,
    IO_D2 ,
    IO_D3 ,
    IO_D4 ,
    IO_D5 ,
    IO_D6 ,
    IO_D7 ,
    IO_D8 ,
    IO_D9 ,
    IO_D10,
    IO_D11,
    IO_D12,
    IO_D13,
    IO_D14,
    IO_D15,
    IO_E0 ,
    IO_E1 ,
    IO_E2 ,
    IO_E3 ,
    IO_E4 ,
    IO_E5 ,
    IO_E6 ,
    IO_E7 ,
    IO_E8 ,
    IO_E9 ,
    IO_E10,
    IO_E11,
    IO_E12,
    IO_E13,
    IO_E14,
    IO_E15			// Dummy Port
} GPIO_ENUM;

#endif  //__GPIO_TYPEDEF__



#ifndef _GPIO_DRVING_DEF_
#define _GPIO_DRVING_DEF_

typedef enum {
    IOA_DRV_4mA=0x0,
    IOA_DRV_8mA=0x1,
/* IOB Driving Options */
    IOB_DRV_4mA=0x0,
    IOB_DRV_8mA=0x1,
/* IOC Driving Options */    
    IOC_DRV_4mA=0x0,
    IOC_DRV_8mA=0x1,
/* IOD Driving Options */
    IOD_DRV_4mA=0x0,
    IOD_DRV_8mA=0x1,
    IOD_DRV_12mA=0x2,
    IOD_DRV_16mA=0x3,
/* IOE Driving Options */    
    IOE_DRV_4mA=0x0,
    IOE_DRV_8mA=0x1
} IO_DRV_LEVEL;


#endif //_GPIO_DRVING_DEF_


extern void gpio_init(void);
extern BOOLEAN gpio_read_io(INT32U port);
extern BOOLEAN gpio_write_io(INT32U port, BOOLEAN data);
extern BOOLEAN gpio_init_io(INT32U port, BOOLEAN direction);
extern BOOLEAN gpio_set_port_attribute(INT32U port, BOOLEAN attribute);
extern BOOLEAN gpio_get_dir(INT32U port);
extern BOOLEAN gpio_drving_init_io(GPIO_ENUM port, IO_DRV_LEVEL gpio_driving_level);
extern void gpio_drving_init(void);
extern void gpio_drving_uninit(void);
extern void gpio_set_ice_en(BOOLEAN status);
extern void gpio_IOE_switch_config_from_HDMI_to_GPIO(void);

#endif /* __drv_l1_GPIO_H__ */
