/*
* Purpose: Driver layer2 for controlling SD/MMC cards
*
* Author: Tristan
*
* Date: 2008/12/11
*
* Copyright Generalplus Corp. ALL RIGHTS RESERVED.
*
* Version : 1.06
*/

#include "drv_l2_sd.h"

//#if (defined _DRV_L2_SD) && (_DRV_L2_SD == 1)

#define C_SD_L2_INIT_RETRY							2

// Timeout value(unit is ms). Maximum timeout value for card initialization is 1 second
#define C_SD_L2_CARD_INIT_TIMEOUT					400		// Some cards need 360 ms to response to ACMD41
#define C_SD_L2_CARD_INIT_RETRY_TIMEOUT				1000	// Some cards need 900 ms to initialize after receiving CMD0
#define C_SD_L2_CARD_PROGRAM_TIMEOUT				750		// 750 ms timeout value for writing data to SD card

// Timeout value(unit is 2.67us)
#define C_SD_L2_CARD_BUSY_TIMEOUT					2500
#define C_SD_L2_SINGLE_READ_CMD_COMPLETE_TIMEOUT	560
#define C_SD_L2_MULTI_READ_CMD_COMPLETE_TIMEOUT		4300
#define C_SD_L2_SINGLE_WRITE_CMD_COMPLETE_TIMEOUT	100
#define C_SD_L2_MULTI_WRITE_CMD_COMPLETE_TIMEOUT	100
#define C_SD_L2_SINGLE_WRITE_DATA_COMPLETE_TIMEOUT	100
#define C_SD_L2_MULTI_WRITE_DATA_COMPLETE_TIMEOUT	100
#define C_SD_L2_ADTC_DATA_FULL_TIMEOUT				3400
#define C_SD_L2_RESP_R136_FULL_TIMEOUT				2000
#define C_SD_L2_RESP_R2_FULL_TIMEOUT				180
#define C_SD_L2_READ_CONTROLLER_STOP_TIMEOUT		3600
#define C_SD_L2_WRITE_CONTROLLER_STOP_TIMEOUT		100

#define TOTAL_SD_CONTROLLER 2

static SD_CARD_STATE_ENUM sd_card_state[TOTAL_SD_CONTROLLER] = {SD_CARD_STATE_INACTIVE, SD_CARD_STATE_INACTIVE};
static INT32U sd_card_rca[TOTAL_SD_CONTROLLER];			// 16 bits (31-16)
static INT32U sd_card_csd[TOTAL_SD_CONTROLLER][4];		// 128 bits
static INT32U sd_card_scr[TOTAL_SD_CONTROLLER][2];		// 64 bits
static INT32U sd_card_cid[TOTAL_SD_CONTROLLER][4];		// 128 bits
static INT32U sd_card_ocr[TOTAL_SD_CONTROLLER];			// 32 bits
static INT32U sd_card_total_sector[TOTAL_SD_CONTROLLER];
//static INT32U sd_card_speed[TOTAL_SD_CONTROLLER];
static INT8U sd_card_type[TOTAL_SD_CONTROLLER];
static INT8U sd_card_bus_width[TOTAL_SD_CONTROLLER];
static INT8U sd_card_protect[TOTAL_SD_CONTROLLER];
static INT8U sd_card_long_polling[TOTAL_SD_CONTROLLER];

static INT32S drvl2_sd_bc_command_set(INT32S SD_DEVICE_NUM, INT32U command, INT32U argument);
static INT32S drvl2_sd_bcr_ac_command_set(INT32S SD_DEVICE_NUM, INT32U command, INT32U argument, INT32U response_type, INT32U *response);
static INT32S drvl2_sd_adtc_command_set(INT32S SD_DEVICE_NUM, INT32U command, INT32U rca, INT32U count, INT32U *response);
static INT32S drvl2_v1_sd_card_initiate(INT32S SD_DEVICE_NUM, INT32U timeout);
static INT32S drvl2_v2_sd_card_initiate(INT32U timeout);
static INT32S drvl2_mmc_card_initiate(INT32U timeout);
static INT32S drvl2_sdc_card_busy_wait(INT32S SD_DEVICE_NUM, INT32U timeout);

INT32S drvl2_sd_init(void)
{
    INT32S ret;
    INT32U response;
    INT32U count;
    INT32U timeout;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    if (sd_card_state[SD_DEVICE_NUM] == SD_CARD_STATE_TRANSFER)
    {
        drvl1_sdc_enable(SD_DEVICE_NUM);

        // If card exists and in transfer state, then just return
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD13, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response) == 0)
        {
            if ((response & 0x1E00) == 0x800)  		// 0x800 is transfer state
            {
                return 0;
            }
        }
    }

    sd_card_rca[SD_DEVICE_NUM] = 0x0;
    sd_card_type[SD_DEVICE_NUM] = C_MEDIA_V1_STANDARD_SD_CARD;
    sd_card_total_sector[SD_DEVICE_NUM] = 0;
//	sd_card_speed[SD_DEVICE_NUM] = 400000;				// 400K Hz
    sd_card_bus_width[SD_DEVICE_NUM] = 1;
    sd_card_protect[SD_DEVICE_NUM] = 0;
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_IDLE;
    sd_card_long_polling[SD_DEVICE_NUM] = 0;	

    drvl1_sdc_init(SD_DEVICE_NUM);
    drvl1_sdc_enable(SD_DEVICE_NUM);

    // Send 74 clock and then issue comand 0
    for (count=0; count<C_SD_L2_INIT_RETRY; count++)
    {
        drvl1_sdc_card_init_74_cycles(SD_DEVICE_NUM);
        drvl2_sd_bc_command_set(SD_DEVICE_NUM, C_SD_CMD0, 0x0);
        drvl1_sdc_card_init_74_cycles(SD_DEVICE_NUM);
        drvl2_sd_bc_command_set(SD_DEVICE_NUM, C_SD_CMD0, 0x0);
        drvl1_sdc_card_init_74_cycles(SD_DEVICE_NUM);

        if (count == 0)
        {
            timeout = C_SD_L2_CARD_INIT_TIMEOUT;
        }
        else
        {
            timeout = C_SD_L2_CARD_INIT_RETRY_TIMEOUT;
        }
        // Issue command 8
        ret = drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM,C_SD_CMD8, 0x000001AA, C_RESP_R7, &response);
        // If the card responses to command 8
        if (ret == 0)  			// Ver 2.0 standard SD card or SDHC card
        {
            if (!(response & 0x100))
            {
                // If response is not valid, it is an unusable card
                sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_INACTIVE;

                return -1;
            }
            if (drvl2_v2_sd_card_initiate(timeout) == 0)
            {
                break;
            }

        }
        else  	// If the card doesn't response to command 8, it is a ver 1.x SD card or MMC
        {
            if (drvl2_v1_sd_card_initiate(SD_DEVICE_NUM,timeout)==0)
            {
                break;
            }
            if (sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMC_CARD)
            {
                if (drvl2_mmc_card_initiate(timeout) == 0)
                {
                    break;
                }
            }
            // if card type is SDHC but fail to response to CMD8, we have to run the initialization process again
        }
#if _OPERATING_SYSTEM != _OS_NONE
        OSTimeDly(1);
#endif
    }
    if (count == C_SD_L2_INIT_RETRY)
    {
        return -1;
    }
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_READY;

    // Send command 2
    if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD2, 0x0, C_RESP_R2, &sd_card_cid[SD_DEVICE_NUM][0]))
    {
        return -1;
    }
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_IDENTIFICATION;

    // Send command 3
    if ((sd_card_type[SD_DEVICE_NUM] != C_MEDIA_MMC_CARD)&&(sd_card_type[SD_DEVICE_NUM] != C_MEDIA_MMCHC_CARD))
    {
        // Send CMD3 and read new RCA, SD will generate RCA itself
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD3, 0x0, C_RESP_R6, &response))
        {
            return -1;
        }
        sd_card_rca[SD_DEVICE_NUM] = response & 0xFFFF0000;
    }
    else
    {
        // Send CMD3 to set a new RCA to MMC card
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD3, 0xFFFF0000, C_RESP_R6, &response))
        {
            return -1;
        }
        sd_card_rca[SD_DEVICE_NUM] = 0xFFFF0000;
    }
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_STANDBY;
    // Set bus width and clock speed
#ifdef GPCV1248_FPGA
    // FPGA 沒有跑這麼快
    //ret = drvl2_sd_bus_clock_set(SD_DEVICE_NUM,12000000);	  // for FPGA
    ret = drvl2_sd_bus_clock_set(3000000);	// for FPGA
    //ret = drvl2_sd_bus_clock_set(SD_DEVICE_NUM,300000);   // for FPGA
#else
    ret = drvl2_sd_bus_clock_set(50000000);    // for EVB
    if(ret < 0)
    {
        ret = drvl2_sd_bus_clock_set(25000000);
    }
#endif

    return ret;
}

static INT32S drvl2_sd_bc_command_set(INT32S SD_DEVICE_NUM, INT32U command, INT32U argument)
{
    return drvl1_sdc_command_send(SD_DEVICE_NUM, command, argument);
}

static INT32S drvl2_sd_bcr_ac_command_set(INT32S SD_DEVICE_NUM, INT32U command, INT32U argument, INT32U response_type, INT32U *response)
{
    INT8U i;

    if (drvl1_sdc_command_send(SD_DEVICE_NUM, command, argument))
    {
        return -1;
    }

    switch (response_type)
    {
        case C_RESP_R1:
        case C_RESP_R1B:
        case C_RESP_R3:
        case C_RESP_R6:
            return drvl1_sdc_response_get(SD_DEVICE_NUM, response, C_SD_L2_RESP_R136_FULL_TIMEOUT);

        case C_RESP_R2:
            for (i=0; i<4; i++)
            {
                if (drvl1_sdc_response_get(SD_DEVICE_NUM, response, C_SD_L2_RESP_R2_FULL_TIMEOUT))
                {
                    return -1;
                }
                response++;
            }

        case C_RESP_R0:
        default:
            break;
    }

    return 0;
}

static INT32S drvl2_sd_adtc_command_set(INT32S SD_DEVICE_NUM, INT32U command, INT32U rca, INT32U count, INT32U *response)
{
    // Clear SD RX data register before read command is issued
    drvl1_sdc_clear_rx_data_register(SD_DEVICE_NUM);

    if (drvl1_sdc_command_send(SD_DEVICE_NUM, command, rca))
    {
        return -1;
    }

    while (count)
    {
        if (drvl1_sdc_data_get(SD_DEVICE_NUM, response, C_SD_L2_ADTC_DATA_FULL_TIMEOUT))
        {
            return -1;
        }
        response++;
        count--;
    }

    return 0;
}

static INT32S drvl2_v1_sd_card_initiate(INT32S SD_DEVICE_NUM, INT32U timeout)
{
    INT32S ret;
    INT32U response;
    INT32U timer;

#if _OPERATING_SYSTEM != _OS_NONE
    timer = OSTimeGet();		// 10ms per tick
#else
    timer = 0;
#endif
    do
    {
        // Send ACMD41 with HCS=0 until card is ready or timeout
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD55, 0x0, C_RESP_R1, &response))
        {
            // If CMD55 timeout occurs, maybe it is a MMC card
            sd_card_type[SD_DEVICE_NUM] = C_MEDIA_MMC_CARD;

            return -1;
        }
        ret = drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_ACMD41, 0x00200000, C_RESP_R3, &sd_card_ocr[SD_DEVICE_NUM]);
        if (ret)
        {
#if _OPERATING_SYSTEM != _OS_NONE
            if ((OSTimeGet()-timer)*10 > timeout)
            {
                // If card timeout occurs, maybe it is a MMC card
                sd_card_type[SD_DEVICE_NUM] = C_MEDIA_MMC_CARD;

                return -1;
            }
            OSTimeDly(1);

#else
            if (++timer > timeout)
            {
                // If card timeout occurs, maybe it is a MMC card
                sd_card_type[SD_DEVICE_NUM] = C_MEDIA_MMC_CARD;

                return -1;
            }

#endif

        }
        else if (!(sd_card_ocr[SD_DEVICE_NUM] & 0x80000000))
        {
#if _OPERATING_SYSTEM != _OS_NONE
            if ((OSTimeGet()-timer)*10 > timeout)
            {
                // If card busy timeout occurs, maybe it is a SDHC card
                sd_card_type[SD_DEVICE_NUM] = C_MEDIA_SDHC_CARD;

                return -1;
            }
            OSTimeDly(1);

#else
            if (++timer > timeout)
            {
                // If card busy timeout occurs, maybe it is a SDHC card
                sd_card_type[SD_DEVICE_NUM] = C_MEDIA_SDHC_CARD;

                return -1;
            }
#endif

        }
        else  		// It is a Ver1.X standard SD card
        {
            sd_card_type[SD_DEVICE_NUM] = C_MEDIA_V1_STANDARD_SD_CARD;
            break;
        }
    }
    while (1) ;

    return 0;
}

static INT32S drvl2_v2_sd_card_initiate(INT32U timeout)
{
    INT32S ret;
    INT32U response;
    INT32U timer;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

#if _OPERATING_SYSTEM != _OS_NONE
    timer = OSTimeGet();		// 10ms per tick
#else
    timer = 0;
#endif
    do
    {
        // Send ACMD41 with HCS=1 until card is ready or timeout
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD55, 0x0, C_RESP_R1, &response))
        {
            return -1;
        }
        ret = drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_ACMD41, 0x40200000, C_RESP_R3, &sd_card_ocr[SD_DEVICE_NUM]);
        if (ret || !(sd_card_ocr[SD_DEVICE_NUM] & 0x80000000))
        {
            // Maximum timeout value for card initialization is 1 second
#if _OPERATING_SYSTEM != _OS_NONE
            if ((OSTimeGet()-timer)*10 > timeout)
            {
                // If card timeout occurs, it is an unusable card
                return -1;
            }
            OSTimeDly(1);

#else
            if (++timer > timeout)
            {
                // If card timeout occurs, it is an unusable card
                return -1;
            }
#endif

        }
        else
        {
            if (sd_card_ocr[SD_DEVICE_NUM] & 0x40000000)  	// If CCS in sd_card_ocr is 1, it is a Ver2.0 SDHC card
            {
                sd_card_type[SD_DEVICE_NUM] = C_MEDIA_SDHC_CARD;
            }
            else  		// If CCS in sd_card_ocr is 0, it is a Ver2.0 standard SD card
            {
                sd_card_type[SD_DEVICE_NUM] = C_MEDIA_V2_STANDARD_SD_CARD;
            }
            break;
        }
    }
    while (1) ;

    return 0;
}

static INT32S drvl2_mmc_card_initiate(INT32U timeout)
{
    INT32S ret;
    INT32U response;
    INT32U timer;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    // Start MMC initialization process
#if _OPERATING_SYSTEM != _OS_NONE
    timer = OSTimeGet();		// 10ms per tick
#else
    timer = 0;
#endif
    do
    {
        // Send CMD1 with voltage range set until card is ready or timeout
        ret = drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD1, 0x00FF8000, C_RESP_R3, &response);
        if (ret || !(response & 0x80000000))
        {
#if _OPERATING_SYSTEM != _OS_NONE
            if ((OSTimeGet()-timer)*10 > timeout)
            {
                // If card timeout occurs, it is an unusable card
                sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_INACTIVE;

                return -1;
            }
            OSTimeDly(1);
#else
            if (++timer > timeout)
            {
                // If card timeout occurs, it is an unusable card
                sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_INACTIVE;

                return -1;
            }
#endif

        }
        else
        {
            // Check whether voltage is acceptable
            if (!(response & 0x00FF8000))
            {
                // If response is not valid, it is an unusable card
                sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_INACTIVE;

                return -1;
            }
            sd_card_type[SD_DEVICE_NUM] = C_MEDIA_MMC_CARD;
            break;
        }
    }
    while (1) ;
    // MMC HC card
    if(response&0x40000000)
        sd_card_type[SD_DEVICE_NUM] = C_MEDIA_MMCHC_CARD;
    return 0;
}


const INT8U speed_table[] = {10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
INT32S drvl2_sd_bus_clock_set(INT32U limit_speed)
{
    INT32U response;
    INT32U c_size, mult, block_len;
    INT32U tran_unit, time_value;
    INT32U max_speed;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    // Make sure that card is ready and in standby state
    if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD13, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response))
    {
        return -1;
    }
    response &= 0x1F00;
    if (response != 0x0700)  		// 0x0700 is standby state
    {
        if (response == 0x0900)  		// 0x800 is transfer state
        {
            if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD7, 0x0, C_RESP_R0, &response))
            {
                return -1;
            }
            if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD13, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response))
            {
                return -1;
            }
            if ((response & 0x1F00) != 0x0700)
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        // Check whether this card supports high speed mode
        if ((sd_card_type[SD_DEVICE_NUM]==C_MEDIA_SDHC_CARD || sd_card_type[SD_DEVICE_NUM]==C_MEDIA_V2_STANDARD_SD_CARD) && limit_speed>25000000 && sd_card_bus_width[SD_DEVICE_NUM]==1)
        {
            INT32U switch_function_data[16];		// 512 bits

            // Make sure we are in transfer state. CMD6 is valid under transfer state only
            if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD7, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1B, &response))
            {
                return -1;
            }

            // Set block length to 64 bytes so that we can read status data of CMD6
            drvl1_sdc_block_len_set(SD_DEVICE_NUM,64);

            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD6, 0x00FFFF01, 16, &switch_function_data[0]) == 0)
            {
                // Check whether card supports high-speed mode
                if ((switch_function_data[3] & 0x00000300) == 0x00000300)
                {
                    drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD6, 0x80FFFF01, 16, &switch_function_data[0]);
                }
            }

            // Restore sector length
            drvl1_sdc_block_len_set(SD_DEVICE_NUM, C_SD_SECTOR_SIZE);

            // Switch back to standby state
            drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD7, 0x0, C_RESP_R0, &response);
        }
    }

    // Send CMD9 and read CSD Register
    if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD9, sd_card_rca[SD_DEVICE_NUM], C_RESP_R2, &sd_card_csd[SD_DEVICE_NUM][0]))
    {
        return -1;
    }

    // Calculate Totoal Memory Size
    if (sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)
    {
        c_size = ((sd_card_csd[SD_DEVICE_NUM][2]>>16) & 0x0000FFFF)+((sd_card_csd[SD_DEVICE_NUM][1]&0x0000003F)<<16);
        sd_card_total_sector[SD_DEVICE_NUM] = (c_size+1) << 10;
    }
    else
    {
        block_len = (sd_card_csd[SD_DEVICE_NUM][1] >> 16) & 0xF;
        c_size = ((sd_card_csd[SD_DEVICE_NUM][1] & 0x3FF) << 2) | ((sd_card_csd[SD_DEVICE_NUM][2] >> 30) & 0x3);
        mult = (sd_card_csd[SD_DEVICE_NUM][2] >> 15) & 0x7;
        sd_card_total_sector[SD_DEVICE_NUM] = (c_size + 1) << (mult + 2 + block_len - 9);
    }

    time_value = (sd_card_csd[SD_DEVICE_NUM][0] & 0x00000078) >> 3;
    tran_unit = sd_card_csd[SD_DEVICE_NUM][0] & 0x00000007;
    if (time_value>0 && time_value<=15)
    {
        max_speed = (INT32U) (speed_table[time_value-1]);
        if (tran_unit == 0)
        {
            max_speed = max_speed * 1000000;
        }
        else if (tran_unit == 1)
        {
            max_speed = max_speed * 100000;
        }
        else if (tran_unit == 2)
        {
            max_speed = max_speed * 1000000;
        }
        else if (tran_unit == 3)
        {
            max_speed = max_speed * 10000000;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
    if (max_speed > limit_speed)
    {
        max_speed = limit_speed;
    }
//	sd_card_speed[SD_DEVICE_NUM] = max_speed;
    if (drvl1_sdc_clock_set(SD_DEVICE_NUM, max_speed))
    {
        return -1;
    }

    // Change to transfer state
    if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD7, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1B, &response))
    {
        return -1;
    }
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_TRANSFER;

    // Only SD has 4 bits mode
    #if GPDV_BOARD_VERSION != GPCV1248_MINI
    if (sd_card_type[SD_DEVICE_NUM] != C_MEDIA_MMC_CARD)
    {
        // Set block length to 8 bytes so that we can read SCR
        if (drvl1_sdc_block_len_set(SD_DEVICE_NUM,8))
        {
            return -1;
        }
        // Send ACMD51 to read 64-bits SD configuration register(SCR)
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD55, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response))
        {
            return -1;
        }

        if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_ACMD51, sd_card_rca[SD_DEVICE_NUM], 2, &sd_card_scr[SD_DEVICE_NUM][0]))
        {
            return -1;
        }
        // Reset block length to C_SD_SECTOR_SIZE bytes
        if (drvl1_sdc_block_len_set(SD_DEVICE_NUM, C_SD_SECTOR_SIZE))
        {
            return -1;
        }

        if (sd_card_scr[SD_DEVICE_NUM][0] & 0x00000400)  		// Check whether this SD card supports 4-bits bus width
        {
            sd_card_bus_width[SD_DEVICE_NUM] = 4;

            // Send ACMD6 to set the card to 4-bit bus width
            if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD55, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response))
            {
                return -1;
            }
            if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_ACMD6, 0x2, C_RESP_R1, &response))
            {
                return -1;
            }

            drvl1_sdc_bus_width_set(SD_DEVICE_NUM, 4);			// Set bus width to 4 bits
        }
    }
	#endif
	
    drvl1_sdc_block_len_set(SD_DEVICE_NUM, C_SD_SECTOR_SIZE);

    return 0;
}

void drvl2_sdc_card_info_get(SD_CARD_INFO_STRUCT *sd_info)
{
    INT8U i;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    sd_info->rca = (sd_card_rca[SD_DEVICE_NUM] >> 16) & 0xFFFF;
    for (i=0; i<4; i++)
    {
        sd_info->csd[i] = sd_card_csd[SD_DEVICE_NUM][i];
    }
    for (i=0; i<2; i++)
    {
        sd_info->scr[i] = sd_card_scr[SD_DEVICE_NUM][i];
    }
    for (i=0; i<4; i++)
    {
        sd_info->cid[i] = sd_card_cid[SD_DEVICE_NUM][i];
    }
    sd_info->ocr = sd_card_ocr[SD_DEVICE_NUM];
}

INT8U drvl2_sdc_card_protect_get(INT32S SD_DEVICE_NUM)
{
    return sd_card_protect[SD_DEVICE_NUM];
}

void drvl2_sdc_card_protect_set(INT32S SD_DEVICE_NUM, INT8U value)
{
    sd_card_protect[SD_DEVICE_NUM] = value;
}

INT32U drvl2_sd_sector_number_get(void)
{
    return sd_card_total_sector[SD_USED_NUM];
}

INT32S drvl2_sdc_card_long_polling_set(INT32S SD_DEVICE_NUM, INT32U status)
{
    sd_card_long_polling[SD_DEVICE_NUM] = status;
    return 0;
}

#define SD_THRESHOLD 1024
INT32S drvl2_sdc_card_busy_wait(INT32S SD_DEVICE_NUM, INT32U timeout)
{
    INT32U timer;
    INT32U cnt;

    // Wait operation complete and back to transfer state
    //#if (defined _DRV_L1_TIMER) && (_DRV_L1_TIMER == 1)
#if 0		// modify by josephhsieh@140225	There is no function called "sw_timer_get_counter_L"
    timer = (INT32U) sw_timer_get_counter_L();
#elif _OPERATING_SYSTEM != _OS_NONE
    timer = OSTimeGet();
#else
    timer = 0;
#endif


    if (sd_card_long_polling[SD_DEVICE_NUM]==0)
        cnt = 0;
    else cnt = SD_THRESHOLD;//- 8;

    do
    {
        drvl1_sdc_card_init_74_cycles(SD_DEVICE_NUM);
        if (drvl1_sdc_card_busy_wait(SD_DEVICE_NUM, 0))
        {
#if _OPERATING_SYSTEM != _OS_NONE
            if ((OSTimeGet()-timer) > (timeout/10))
            {
                return -1;
            }
            else
            {
                if (cnt>=SD_THRESHOLD)  
                {
					#if WIFI_FUNC_ENABLE
                		OSTimeDly(2);
                	#else
                		OSTimeDly(1);
                	#endif
                }
                else
                {
                	cnt++;
                }
            }
#else
            if (++timer > timeout)
            {
                return -1;
            }
#endif
        }
        else
        {
            break;
        }
    }
    while (1) ;

    return 0;
}

INT32S drvl2_sd_read(INT32U start_sector, INT32U *buffer, INT32U sector_count)
{
    INT32U response;
    INT32S ret;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;


    if (sd_card_state[SD_DEVICE_NUM] != SD_CARD_STATE_TRANSFER)
    {
        return -1;
    }
    if (drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_BUSY_TIMEOUT))
    {
        return -1;
    }
#if 0
    if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD13, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response)==0)
    {
        // Check whether card is in transfer state
        if (!(response & 0x100))
        {
            return -1;
        }
    }
#endif


    // Clear SD RX data register before read command is issued
    drvl1_sdc_clear_rx_data_register(SD_DEVICE_NUM);

    if (sector_count == 1)
    {
        if ((sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)||(sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMCHC_CARD))
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD17, start_sector, 0, &response))
            {
                return -1;
            }
        }
        else
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD17, start_sector<<9, 0, &response))
            {
                return -1;
            }
        }
        sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_SENDING_DATA;

        ret = drvl1_sdc_read_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, NULL);


        if (drvl1_sdc_data_crc_status_get(SD_DEVICE_NUM))
        {
            ret = -1;
        }
        if (drvl1_sdc_command_complete_wait(SD_DEVICE_NUM, C_SD_L2_SINGLE_READ_CMD_COMPLETE_TIMEOUT))
        {
            ret = -1;
        }

    }
    else
    {
        if ((sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)||(sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMCHC_CARD))
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD18, start_sector, 0, &response))
            {
                return -1;
            }
        }
        else
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD18, start_sector<<9, 0, &response))
            {
                return -1;
            }
        }
        sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_SENDING_DATA;

        ret = drvl1_sdc_read_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, NULL);

        if (drvl1_sdc_data_crc_status_get(SD_DEVICE_NUM))
        {
            ret = -1;
        }
        if (drvl1_sdc_command_complete_wait(SD_DEVICE_NUM, C_SD_L2_MULTI_READ_CMD_COMPLETE_TIMEOUT))
        {
            ret = -1;
        }

        if (drvl1_sdc_stop_controller(SD_DEVICE_NUM, C_SD_L2_READ_CONTROLLER_STOP_TIMEOUT))
        {
            ret = -1;
        }

        drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD12, 0x0, C_RESP_R1B, &response);
    }
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_TRANSFER;

    return ret;
}

INT32S drvl2_sd_write(INT32U start_sector, INT32U *buffer, INT32U sector_count)
{
    INT32U response;
    INT32S ret;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    if (sd_card_state[SD_DEVICE_NUM] != SD_CARD_STATE_TRANSFER)
    {
        return -1;
    }
    if (drvl2_sdc_card_protect_get(SD_DEVICE_NUM))
    {
        return -1;
    }
    if (drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_BUSY_TIMEOUT))
    {
        return -1;
    }

    if (sector_count == 1)
    {
        if ((sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)||(sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMCHC_CARD))
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD24, start_sector, 0, &response))
            {
                return -1;
            }
        }
        else
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD24, start_sector<<9, 0, &response))
            {
                return -1;
            }
        }
        if (drvl1_sdc_command_complete_wait(SD_DEVICE_NUM, C_SD_L2_SINGLE_WRITE_CMD_COMPLETE_TIMEOUT))
        {
            return -1;
        }
        sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_RECEIVE_DATA;

        ret = drvl1_sdc_write_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, NULL);

        sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_PROGRAMMING;
        if (drvl1_sdc_data_crc_status_get(SD_DEVICE_NUM))
        {
            ret = -1;
        }
        if (drvl1_sdc_data_complete_wait(SD_DEVICE_NUM, C_SD_L2_SINGLE_WRITE_DATA_COMPLETE_TIMEOUT))
        {
            ret = -1;
        }

    }
    else
    {
        if (sd_card_type[SD_DEVICE_NUM] != C_MEDIA_MMC_CARD)
        {
            // Send ACMD23 to pre-erase block
            if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD55, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response) == 0)
            {
                drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_ACMD23, sector_count, C_RESP_R1, &response);
            }
        }

        if ((sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)||(sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMCHC_CARD))
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD25, start_sector, 0, &response))
            {
                return -1;
            }
        }
        else
        {
            if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD25, start_sector<<9, 0, &response))
            {
                return -1;
            }
        }
        if (drvl1_sdc_command_complete_wait(SD_DEVICE_NUM, C_SD_L2_MULTI_WRITE_CMD_COMPLETE_TIMEOUT))
        {
            return -1;
        }
        sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_RECEIVE_DATA;

        ret = drvl1_sdc_write_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, NULL);

        if (drvl1_sdc_data_crc_status_get(SD_DEVICE_NUM))
        {
            ret = -1;
        }
        if (drvl1_sdc_data_complete_wait(SD_DEVICE_NUM, C_SD_L2_MULTI_WRITE_DATA_COMPLETE_TIMEOUT))
        {
            ret = -1;
        }
        if (drvl1_sdc_stop_controller(SD_DEVICE_NUM, C_SD_L2_WRITE_CONTROLLER_STOP_TIMEOUT))
        {
            ret = -1;
        }

        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD12, 0x0, C_RESP_R1B, &response))
        {
            ret = -1;
        }
        sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_PROGRAMMING;

    }

    // Wait operation complete and back to transfer state
    ret = drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_PROGRAM_TIMEOUT);
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_TRANSFER;

    return ret;
}

INT32S drvl2_sd_card_remove(void)
{
    INT32U response;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    // Reset card to idle state
    if (sd_card_state[SD_DEVICE_NUM] >= SD_CARD_STATE_TRANSFER)
    {
        if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD13, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response))
        {
            sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_INACTIVE;
        }
        else
        {
            if ((response & 0x1E00) < 0x800)
            {
                // If card exists and not in transfer state, send it back to idle state
                drvl2_sd_bc_command_set(SD_DEVICE_NUM, C_SD_CMD0, 0x0);
                drvl2_sd_bc_command_set(SD_DEVICE_NUM, C_SD_CMD0, 0x0);
                drvl2_sd_bc_command_set(SD_DEVICE_NUM, C_SD_CMD0, 0x0);
                sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_IDLE;
            }
            else if ((response & 0x1E00) != 0x800)
            {
                // If card is in sending-data or receive data or programming state, send stop command
                drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD12, 0x0, C_RESP_R1B, &response);

                // Wait until card is not busy before turning off controller
                drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_PROGRAM_TIMEOUT);
            }
        }
    }

    drvl1_sdc_disable(SD_DEVICE_NUM);

    return 0;
}

INT32S drvl2_sd_read_start(INT32U start_sector, INT32U sector_count)
{
    INT32U response;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

//DBG_PRINT("drvl2_sd_read_start()\r\n");

    if (sd_card_state[SD_DEVICE_NUM] != SD_CARD_STATE_TRANSFER)
    {
        return -1;
    }
    if (drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_BUSY_TIMEOUT))
    {
        return -1;
    }

    // Clear SD RX data buffer before read command is issued
    drvl1_sdc_clear_rx_data_register(SD_DEVICE_NUM);

    if ((sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)||(sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMCHC_CARD))
    {
        if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD18, start_sector, 0, &response))
        {
            return -1;
        }
    }
    else
    {
        if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD18, start_sector<<9, 0, &response))
        {
            return -1;
        }
    }

    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_SENDING_DATA;

    return 0;
}

volatile INT8S sd_read_sector_result[TOTAL_SD_CONTROLLER];
INT32S drvl2_sd_read_sector(INT32U *buffer, INT32U sector_count, INT8U wait_flag)
{
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    //DBG_PRINT("drvl2_sd_read_sector()\r\n");

    if (wait_flag == 0)  	// Start DMA and return immediately
    {
        return drvl1_sdc_read_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, (INT8S *) &sd_read_sector_result[SD_DEVICE_NUM]);
    }
    else if (wait_flag == 1)  	// Start DMA and wait until done
    {
        return drvl1_sdc_read_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, NULL);
    }

    // Query status and return when done
    while (sd_read_sector_result[SD_DEVICE_NUM] == C_DMA_STATUS_WAITING) ;
    if (sd_read_sector_result[SD_DEVICE_NUM] != C_DMA_STATUS_DONE)
    {
        return -1;
    }

    return 0;
}

INT32S drvl2_sd_read_stop(void)
{
    INT32U response;
    INT32S ret;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    ret = 0;
    if (drvl1_sdc_data_crc_status_get(SD_DEVICE_NUM))
    {
        ret = -1;
    }
    if (drvl1_sdc_command_complete_wait(SD_DEVICE_NUM, C_SD_L2_MULTI_READ_CMD_COMPLETE_TIMEOUT))
    {
        ret = -1;
    }
    if (drvl1_sdc_stop_controller(SD_DEVICE_NUM, C_SD_L2_READ_CONTROLLER_STOP_TIMEOUT))
    {
        ret = -1;
    }

    drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD12, 0x0, C_RESP_R1B, &response);
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_TRANSFER;

    return ret;
}

INT32S drvl2_sd_write_start(INT32U start_sector, INT32U sector_count)
{
    INT32U response;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    if (sd_card_state[SD_DEVICE_NUM] != SD_CARD_STATE_TRANSFER)
    {
        return -1;
    }
    if (drvl2_sdc_card_protect_get(SD_DEVICE_NUM))
    {
        return -1;
    }
    if (drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_BUSY_TIMEOUT))
    {
        return -1;
    }

    if ((sd_card_type[SD_DEVICE_NUM] == C_MEDIA_SDHC_CARD)||(sd_card_type[SD_DEVICE_NUM] == C_MEDIA_MMCHC_CARD))
    {
        if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD25, start_sector, 0, &response))
        {
            return -1;
        }
    }
    else
    {
        if (drvl2_sd_adtc_command_set(SD_DEVICE_NUM, C_SD_CMD25, start_sector<<9, 0, &response))
        {
            return -1;
        }
    }

    if (drvl1_sdc_command_complete_wait(SD_DEVICE_NUM, C_SD_L2_MULTI_WRITE_CMD_COMPLETE_TIMEOUT))
    {
        return -1;
    }
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_RECEIVE_DATA;

    return 0;
}

static volatile INT8S sd_write_sector_result[TOTAL_SD_CONTROLLER];
INT32S drvl2_sd_write_sector(INT32U *buffer, INT32U sector_count, INT8U wait_flag)
{
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;
    if (wait_flag == 0)  	// Start DMA and return immediately
    {
        return drvl1_sdc_write_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, (INT8S *) &sd_write_sector_result[SD_DEVICE_NUM]);
    }
    else if (wait_flag == 1)  	// Start DMA and wait until done
    {
        return drvl1_sdc_write_data_by_dma(SD_DEVICE_NUM, buffer, sector_count, NULL);
    }

    // Query status and return when done
    while (sd_write_sector_result[SD_DEVICE_NUM] == C_DMA_STATUS_WAITING) ;
    if (sd_write_sector_result[SD_DEVICE_NUM] != C_DMA_STATUS_DONE)
    {
        return -1;
    }

    return 0;
}

INT32S drvl2_sd_write_stop(void)
{
    INT32U response;
    INT32S ret;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    ret = 0;
    if (drvl1_sdc_data_complete_wait(SD_DEVICE_NUM, C_SD_L2_MULTI_WRITE_DATA_COMPLETE_TIMEOUT))
    {
        ret = -1;
    }
    if (drvl1_sdc_data_crc_status_get(SD_DEVICE_NUM))
    {
        ret = -1;
    }
    if (drvl1_sdc_stop_controller(SD_DEVICE_NUM, C_SD_L2_WRITE_CONTROLLER_STOP_TIMEOUT))
    {
        ret = -1;
    }

    drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD12, 0x0, C_RESP_R1B, &response);
    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_PROGRAMMING;

    // Wait operation complete and back to transfer state
    ret = drvl2_sdc_card_busy_wait(SD_DEVICE_NUM, C_SD_L2_CARD_PROGRAM_TIMEOUT);

    sd_card_state[SD_DEVICE_NUM] = SD_CARD_STATE_TRANSFER;

    return ret;
}

INT16S drvl2_sdc_live_response(void)
{
    INT32U response;
    INT32S SD_DEVICE_NUM = SD_USED_NUM ;

    if (drvl2_sd_bcr_ac_command_set(SD_DEVICE_NUM, C_SD_CMD13, sd_card_rca[SD_DEVICE_NUM], C_RESP_R1, &response) == 0)
    {
        // Card exist
        return 0;
    }
    else
    {
        // Card not exist
        return -1;
    }
}

//#endif		// _DRV_L2_SD
