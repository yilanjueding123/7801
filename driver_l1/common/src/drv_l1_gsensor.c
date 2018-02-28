#include "drv_l1_gsensor.h"
#include "gplib.h"

#define DA380_SLAVE_ID    0x4E
#define DMARD07_SLAVE_ID  0x38

#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
	 #define G_SlaveAddr DA380_SLAVE_ID
#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)
	#define G_SlaveAddr DMARD07_SLAVE_ID
#endif

static OS_EVENT		*sw_i2c_sem = NULL;

static INT8U G_sensor_status=0;

//====================================================================================================
void I2C_delay (
	INT16U i
) {
	INT16U j;

	for (j=0;j<(i<<4);j++)
		i=i;
}
//============================================
void I2C_start (void)
{
	gpio_write_io(I2C_SCL, DATA_HIGH);					//SCL1
	I2C_delay (1);
	gpio_write_io(I2C_SDA, DATA_HIGH);					//SDA1
	I2C_delay (1);
	gpio_write_io(I2C_SDA, DATA_LOW);					//SDA0
	I2C_delay (1);
}
//===================================================================
void I2C_stop (void)
{
	I2C_delay (1);
	gpio_write_io(I2C_SDA, DATA_LOW);					//SDA0
	I2C_delay (1);
	gpio_write_io(I2C_SCL, DATA_HIGH);					//SCL1
	I2C_delay (1);
	gpio_write_io(I2C_SDA, DATA_HIGH);					//SDA1
	I2C_delay (1);
}


//===================================================================
void I2C_w_phase (INT16U value)
{
	INT16U i;

	for (i=0;i<8;i++)
	{
		gpio_write_io(I2C_SCL, DATA_LOW);					//SCL0
		I2C_delay (1);
		if (value & 0x80)
			gpio_write_io(I2C_SDA, DATA_HIGH);				//SDA1
		else
			gpio_write_io(I2C_SDA, DATA_LOW);				//SDA0
//		sccb_delay (1);
		gpio_write_io(I2C_SCL, DATA_HIGH);					//SCL1
		I2C_delay(1);
		value <<= 1;
	}
	// The 9th bit transmission
	gpio_write_io(I2C_SCL, DATA_LOW);						//SCL0
	gpio_init_io(I2C_SDA, GPIO_INPUT);						//SDA is Hi-Z mode
	I2C_delay(1);
	gpio_write_io(I2C_SCL, DATA_HIGH);						//SCL1
	I2C_delay(1);
	gpio_write_io(I2C_SCL, DATA_LOW);						//SCL0
	gpio_init_io(I2C_SDA, GPIO_OUTPUT);					//SDA is Hi-Z mode
}

//===================================================================
INT16U I2C_r_phase (void)
{
	INT16U i;
	INT16U data;

	gpio_init_io(I2C_SDA, GPIO_INPUT);						//SDA is Hi-Z mode
	data = 0x00;
	for (i=0;i<8;i++)
	{
		gpio_write_io(I2C_SCL, DATA_LOW);					//SCL0
		I2C_delay(1);
		gpio_write_io(I2C_SCL, DATA_HIGH);					//SCL1
		data <<= 1;
		data |=( gpio_read_io(I2C_SDA));
		I2C_delay(1);
	}
	// The 9th bit transmission
	gpio_write_io(I2C_SCL, DATA_LOW);						//SCL0
	gpio_init_io(I2C_SDA, GPIO_OUTPUT);					//SDA is output mode
	gpio_write_io(I2C_SDA, DATA_HIGH);						//SDA0, the nighth bit is NA must be 1
	I2C_delay(1);
	gpio_write_io(I2C_SCL, DATA_HIGH);						//SCL1
	I2C_delay(1);
	gpio_write_io(I2C_SCL, DATA_LOW);						//SCL0
	return data;
}

//====================================================================================================
void I2C_gpio_init (
	INT32U nSCL,			// Clock Port No
	INT32U nSDA				// Data Port No
){
	//init IO
	gpio_set_port_attribute(nSCL, ATTRIBUTE_HIGH);
	gpio_set_port_attribute(nSDA, ATTRIBUTE_HIGH);
	gpio_init_io(nSCL, GPIO_OUTPUT);				//set dir
	gpio_init_io(nSDA, GPIO_OUTPUT);				//set dir
	gpio_write_io(nSCL, DATA_HIGH);					//SCL1
	gpio_write_io(nSDA, DATA_HIGH);					//SDA0
}



void g_sensor_write(INT8U id, INT8U addr, INT8U data)
{
	// 3-Phase write transmission cycle is starting now ...
	I2C_start();									// Transmission start
	I2C_w_phase(id);								// Phase 1: Device ID
	I2C_w_phase(addr);								// Phase 2: Register address. High pass filter enable
	I2C_w_phase(data);								// Phase 3: Data value
	I2C_stop();									// Transmission stop
}

INT16U g_sensor_read(INT8U id, INT8U addr)
{
	INT16U redata;
	
	I2C_start();									// Transmission start
	I2C_w_phase(id);								// Phase 1: Device ID
	I2C_w_phase(addr);								// Phase 2: Register address. Transient source
	I2C_start();									// Transmission start
	I2C_w_phase(id | 0x01);						// Phase 1 (read)
	redata = (INT8U) I2C_r_phase();				// Phase 2
	I2C_stop();									// Transmission stop

   return redata;
}

INT32U G_Get_ACC_Data(INT8U addr, INT8U Num)
{
   INT16U Redata,temp;
   
   if(Num==1){
  
      Redata = g_sensor_read(G_SlaveAddr,addr);
   
   }else{
     temp = g_sensor_read(G_SlaveAddr,addr);
     Redata = g_sensor_read(G_SlaveAddr,addr+1);
     
     Redata = (Redata<<8)|(temp&0x00ff);
   }
  
   return Redata;
}

void sw_i2c_lock(void)
{
	INT8U err;

	OSSemPend(sw_i2c_sem, 0, &err);
}

void sw_i2c_unlock(void)
{
	OSSemPost(sw_i2c_sem);
}

INT8U ap_gsensor_power_on_get_status(void)
{
  return G_sensor_status;
}

void ap_gsensor_power_on_set_status(INT8U status)
{
   if(status)
     G_sensor_status=1;  //gsensor power on
   else
     G_sensor_status=0;  //key power on
}

void G_Sensor_DA380_Init(INT8U level)
{
	INT16U temp1,temp2;

	temp1 = g_sensor_read(G_SlaveAddr,CHIPID);
	DBG_PRINT("chip_id = %x\r\n", temp1);

	temp1 = g_sensor_read(G_SlaveAddr, MOTION_FLAG);
	temp2 = g_sensor_read(G_SlaveAddr, INT_MAP1);
	if((temp1 != 0xff) && (temp1 & 0x04)&&(temp2 !=0)) //active int flag
	{
		ap_gsensor_power_on_set_status(1);
		DBG_PRINT("==========gsensor power on=========\r\n");
	} else {
		ap_gsensor_power_on_set_status(0);
		DBG_PRINT("==========key power on=============\r\n");
	}

	g_sensor_write(G_SlaveAddr,SOFT_RESET,0x24);
	drv_msec_wait(4);
	g_sensor_write(G_SlaveAddr,RESOLUTION_RANGE,0x02);
	g_sensor_write(G_SlaveAddr,MODE_BW,0x1e);
	g_sensor_write(G_SlaveAddr,ODR_AXIS,0x07);

#if USE_G_SENSOR_ACTIVE ==0
	g_sensor_write(G_SlaveAddr,INT_CONFIG,0x04); //selects active level low for pin INT
#else
	g_sensor_write(G_SlaveAddr,INT_CONFIG,0x05);   //selects active level high for pin INT
#endif
	
	g_sensor_write(G_SlaveAddr,INT_LATCH,0x8f);

	g_sensor_write(G_SlaveAddr,INT_SET1,0x07);			//avtive x,y,z interrupt
	g_sensor_write(G_SlaveAddr,ACTIVE_DUR,0x03);
	g_sensor_write(G_SlaveAddr,INT_MAP1,0x00);	//doesn't map to INT
	g_sensor_write(G_SlaveAddr,INT_LATCH, 0x87);		//latch forever

	switch(level)
	{
		case 1://low
		g_sensor_write(G_SlaveAddr,ACTIVE_THS,DMT_SENSITIVE_LOW);
		break;
		case 2://mid
        g_sensor_write(G_SlaveAddr,ACTIVE_THS,DMT_SENSITIVE_MID);
		break;
		case 3://high
        g_sensor_write(G_SlaveAddr,ACTIVE_THS,DMT_SENSITIVE_HIGH);
		break;
	}
}


void DA380_Enter_Interrupt_WakeUp_Mode(INT8U level)
{
    I2C_gpio_init(I2C_SCL,I2C_SDA);

	g_sensor_write(G_SlaveAddr,SOFT_RESET,0x24);
	drv_msec_wait(4);
	g_sensor_write(G_SlaveAddr,RESOLUTION_RANGE,0x02);
	g_sensor_write(G_SlaveAddr,ODR_AXIS,0x07);

#if USE_G_SENSOR_ACTIVE ==0
	g_sensor_write(G_SlaveAddr,INT_CONFIG,0x04); //selects active level low for pin INT
#else
	g_sensor_write(G_SlaveAddr,INT_CONFIG,0x05);   //selects active level high for pin INT
#endif
	g_sensor_write(G_SlaveAddr,INT_LATCH,0x8f);

	g_sensor_write(G_SlaveAddr,INT_SET1,0x07);
	g_sensor_write(G_SlaveAddr,ACTIVE_DUR,0x03);
	switch(level)
	{
		case 1://hig
		g_sensor_write(G_SlaveAddr,ACTIVE_THS,P_SENSITIVE_HIGH);		
		break;
		case 2://mid
        g_sensor_write(G_SlaveAddr,ACTIVE_THS,P_SENSITIVE_MID);
		break;
		case 3://low
        g_sensor_write(G_SlaveAddr,ACTIVE_THS,P_SENSITIVE_LOW);
		break;
	 default: 
	    break; 
	}

	if(level)
	   g_sensor_write(G_SlaveAddr,INT_MAP1, 0x04);//map to INT pin
	 else 
	   g_sensor_write(G_SlaveAddr,INT_MAP1,0x00); //doesn't map to INT
	 
	 
	// g_sensor_write(G_SlaveAddr,MODE_BW,0x1e);  //normal mode
	 g_sensor_write(G_SlaveAddr,MODE_BW,0x5e);    //low power mode
	 g_sensor_write(G_SlaveAddr,INT_LATCH, 0x87); //clear gsensor interrupt flag
	  
}

void DA380_gps_data_get(INT8U *pDATA)
{
	sw_i2c_lock();

	*pDATA++ = g_sensor_read(G_SlaveAddr,  ACC_X_LSB);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  ACC_X_MSB);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  ACC_Y_LSB);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  ACC_Y_MSB);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  ACC_Z_LSB);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  ACC_Z_MSB);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  ORIENT_STATUS);
	*pDATA++ = g_sensor_read(G_SlaveAddr,  RESOLUTION_RANGE);

	sw_i2c_unlock();
}

void DA380_gps_data_set(void *pDST, void *pSRC)
{
	INT16U *s = (INT16U *)pSRC;
	Gsensor_Data *d = (Gsensor_Data *)pDST;

	d->Axis.Xacc = (INT32U )(*s++)>>2;
	d->Axis.Yacc = (INT32U)(*s++)>>2;
	d->Axis.Zacc = (INT32U)(*s++)>>2;
	d->Ori = (INT32U)( ((*s)>>4)&(0x3) );
	d->SH = (INT32U)( (*s)>>6 );
	#if 0
	{
		INT16U config;	
		config = ((*s)>>8)&0x3;
		switch ()
		{
			case :
				break;
			case :
				break;
			case :
				break;
		}
	}
	#endif
}

void ap_gsensor_set_sensitive(INT8U Gsensor)
{
	sw_i2c_lock();
	
#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
	switch(Gsensor)
	   {
		 case 0://doesn't map to INT
		    g_sensor_write(G_SlaveAddr,INT_MAP1,0x00);
		   break;
		 case 1: //2g
		    g_sensor_write(G_SlaveAddr,ACTIVE_THS,DMT_SENSITIVE_HIGH);	
		   break; 
		 case 2://4g
		   g_sensor_write(G_SlaveAddr,ACTIVE_THS,DMT_SENSITIVE_MID);	
		   break; 
		 case 3://8g
		    g_sensor_write(G_SlaveAddr,ACTIVE_THS,DMT_SENSITIVE_LOW);	
		   break; 
		 default: 
		  
		   break; 
	  }
	 g_sensor_write(G_SlaveAddr,INT_LATCH, 0x87);	//clear gsensor interrupt flag
	
#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)

  switch(Gsensor)
	{
		case 1://low	
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7,DMT_ARD07_LOW);
			break;
		case 2: 
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7, DMT_ARD07_MID);
			break;
		case 3:
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7,DMT_ARD07_HIGH);
			break;
	}

#endif

	sw_i2c_unlock();
}


INT16U G_sensor_get_int_active(void)
{
    INT16U temp=0xff; 
	sw_i2c_lock();
#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
	
	temp = g_sensor_read(G_SlaveAddr, MOTION_FLAG);
	
#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)
	
	temp = g_sensor_read(G_SlaveAddr,DMT_ARD07_CTRL_REG_6);
#endif	
	sw_i2c_unlock();
    return temp;
}
void G_sensor_clear_int_flag(void)
{
   sw_i2c_lock();
#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
	
	 g_sensor_write(G_SlaveAddr,INT_LATCH, 0x87); //clear gsensor interrupt flag
	
#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)
	
	g_sensor_read(G_SlaveAddr,DMT_ARD07_CTRL_REG_6);
	
#endif	   
   
   sw_i2c_unlock();
}


//=======================================================================
 void G_Sensor_DMARD07_Init(INT8U level)
 {
 
	INT32U ReadValue;
	INT8U  i;

	ReadValue = 0;
	ReadValue = g_sensor_read(G_SlaveAddr,0x0F);
	DBG_PRINT("G sensor Who am I = %02X\r\n", ReadValue);
	
	if(ReadValue != 0x07) {
		for(i=0;i<5;i++)
		{
			ReadValue = g_sensor_read(G_SlaveAddr,0x0F);
	        DBG_PRINT("G sensor Who am I = %02X\r\n", ReadValue);
			if(ReadValue == 0x07)	
			{
				break;
			}
		}
		if(i==5)
		{
			DBG_PRINT("no G sensor\r\n");
		}
	}

	ReadValue =0;
	ReadValue = g_sensor_read(G_SlaveAddr,DMT_ARD07_CTRL_REG_6);
	if((ReadValue) && (ReadValue != 0xff))
	{
	    ap_gsensor_power_on_set_status(1);
		DBG_PRINT("==========gsensor power on=========\r\n");	
	}	
	else	
	{		
		ap_gsensor_power_on_set_status(0);
	    DBG_PRINT("==========key power on=============\r\n");	
	}

	ReadValue = 0;
	ReadValue = g_sensor_read(G_SlaveAddr,0x53);
	DBG_PRINT("G sensor sw reset = %02X\r\n", ReadValue);
	
	ReadValue = 0;
	ReadValue = g_sensor_read(G_SlaveAddr,0x0F);
	DBG_PRINT("G sensor Who am I = %02X\r\n", ReadValue);
	
	// init setting	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_1, 0x27);// Normal Power:342Hz, XYZ enable 
	
	//g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_1, 0x47);// Low Power:32Hz, XYZ enable 

	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_2, 0x24);// 2G mode, High Pass Filter for INT1, Low pass filter for data
	
	//g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_2, 0x14);// 2G mode, High Pass Filter for INT1, High pass filter for data
	//g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_2, 0x10);// 2G mode, Low Pass Filter for INT1, High pass filter for data
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_3, 0x00);//  High-pass Filter Cutoff for 0.6 Hz 
		
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_4, 0x2c);// No latch, INT SRC1 enable, active 1 
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_9, 0x00);
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_5, 0x2A);
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_5, 0x2A);
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7, 0x10);// 0x30 Threshold = 755.9 mg 
	
  switch(level)
	{
		case 1://low	
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7,DMT_ARD07_LOW);
			break;
		case 2: 
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7, DMT_ARD07_MID);
			break;
		case 3:
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7,DMT_ARD07_HIGH);
			break;
	}
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_8, 0x08);//  Duration = 47.1 ms 
	
 }

void DMARD07_Enter_Interrupt_WakeUp_Mode(INT8U level)
{
	INT32U ReadValue;

	ReadValue = g_sensor_read(G_SlaveAddr,0x53);
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_1,0x27); // normal mode, enable interrupt,data rate 342HZ
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_2,0x24);// +-2g data low filter,int 1 source hight filter
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_3,0x00);
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_4,0x64); //ativity high ,latch  SRC1, SRC2 latch disable, int pin to SRC1
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_9,0x00);//auto  Awake function disable

	switch(level)
	{
		case 1://low	
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7,DMT_ARD07_LOW);
			break;
		case 2: 
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7, DMT_ARD07_MID);
			break;
		case 3:
			g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_7,DMT_ARD07_HIGH);
			break;
	}
     if(level)
	   g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_4,0x64);//map to INT pin
	 else 
	   g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_4,0x2c); //doesn't map to INT
	 
	
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_8,0x04);
	g_sensor_write(G_SlaveAddr,DMT_ARD07_CTRL_REG_5,0xea);
	ReadValue = g_sensor_read(G_SlaveAddr,DMT_ARD07_CTRL_REG_4);
	
	DBG_PRINT("end reg[0x47]=%x\n",ReadValue);
}

void G_Sensor_Init(INT8U level)
{
	  if(sw_i2c_sem == NULL) sw_i2c_sem = OSSemCreate(1);

	  G_sensor_status=0;
	  I2C_gpio_init(I2C_SCL,I2C_SDA);
	  
	#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)	
			G_Sensor_DA380_Init(level);
	#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)
			G_Sensor_DMARD07_Init(level);
	#endif
	  
}

void G_Sensor_park_mode_init(INT8U level)
{
	   I2C_gpio_init(I2C_SCL,I2C_SDA);
	  
    #if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
		
	   DA380_Enter_Interrupt_WakeUp_Mode(level);
			
	#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)
	
	   DMARD07_Enter_Interrupt_WakeUp_Mode(level);
		
	#endif
  
}

// for GPS issue
void G_Sensor_gps_data_get(INT8U *pDATA)
{
    	#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
		
	   DA380_gps_data_get(pDATA);
			
	#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)

	DBG_PRINT("Get function is'nt implement yet\r\n");
		
	#endif
}

void G_Sensor_gps_data_set(void *pDST, void *pSRC)
{
    	#if (USE_G_SENSOR_NAME == G_SENSOR_DA380)
		
	   DA380_gps_data_set(pDST, pSRC);
			
	#elif (USE_G_SENSOR_NAME == G_SENSOR_DMARD07)

	DBG_PRINT("Set function is'nt implement yet\r\n");
		
	#endif
}

