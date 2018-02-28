/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#include <cmd_def.h>
#include <log.h>
#include <cli.h>
#include <ssv_lib.h>
#include <os_wrapper.h>
#include <netmgr/net_mgr.h>
#include "ssv_dev.h"
#include "cli_cmd_wifi.h"
#include "cli_cmd_net.h"
#include <ssv_ether.h>

#if(ENABLE_SMART_CONFIG==1)
#include <SmartConfig/SmartConfig.h>
#endif

#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
#define WPA_AUTH_ALG_LEAP BIT(2)
#define WPA_AUTH_ALG_FT BIT(3)

//#define SEC_USE_NONE
//#define SEC_USE_WEP40_PSK
//#define SEC_USE_WEP40_OPEN
//#define SEC_USE_WEP104_PSK
//#define SEC_USE_WEP104_OPEN
//#define SEC_USE_WPA_TKIP
#define SEC_USE_WPA2_CCMP


// iw command executioner
void _cmd_wifi_scan (s32 argc, char *argv[]);
void _sconfig_usage(void);
void _cmd_wifi_sconfig (s32 argc, char *argv[]);
static void _cmd_wifi_join (s32 argc, char *argv[]);
void _cmd_wifi_join_other (s32 argc, char *argv[]);
static void _cmd_wifi_leave (s32 argc, char *argv[]);
static void _cmd_wifi_list (s32 argc, char *argv[]);
void _cmd_wifi_ap (s32 argc, char *argv[]);

#ifdef USE_CMD_RESP
static void _handle_cmd_resp (u32 evt_id, u8 *data, s32 len);
static void _deauth_handler (void *data);
#endif

// CB for host event
static void _host_event_handler(u32 evt_id, void *data, s32 len);
// Individual event handler
static void _scan_down_handler (void *data);
static void _sconfig_scan_done (void *data);
static void _scan_result_handler(void * data);
static void _bus_loopback_handler (void *data);
static void _join_result_handler (void *data);
static void _leave_result_handler (void *data);
static void _get_soc_reg_response(u32 eid, void *data, s32 len);
void _soc_evt_handler_ssv6xxx_log(u32 eid, void *data, s32 len);
#ifndef CONFIG_NO_WPA2
extern void ssv6xxx_eapol_data_reg_cb();
#endif
typedef struct ba_session_st {
	u16 tid;
	u16 buf_size;
	u16 policy;
	u16 timeout;
} BA_SESSION ;

u32 g_cli_ps_lp;
bool g_cli_joining=false;
//static BA_SESSION g_ba_session;

#define ON_OFF_LAG_INTERVAL 1000


/*===================== Start of Get Command Handlers ====================*/


void _soc_evt_get_soc_status(void *data)
{
    char *sta_status[]={ "STA_STATE_UNAUTH_UNASSOC",/*STA_STATE_UNAUTH_UNASSOC*/
                        "STA_STATE_AUTHED_UNASSOC",/*STA_STATE_AUTHED_UNASSOC*/
                        "STA_STATE_AUTHED_ASSOCED",/*STA_STATE_AUTHED_ASSOCED*/
                        "STA_STATE_ASSOCED_4way",/*STA_STATE_ASSOCED_4way*/
                        };
    char *sta_action[]={
                        "STA_ACTION_INIT",/*STA_ACTION_INIT*/
                        "STA_ACTION_IDLE",/*STA_ACTION_IDLE*/
                        "STA_ACTION_READY",/*STA_ACTION_READY*/
                        "STA_ACTION_RUNNING",/*STA_ACTION_RUNNING*/
                        "STA_ACTION_SCANING",/*STA_ACTION_SCANING*/
                        "STA_ACTION_JOING",/*STA_ACTION_JOING*/
                        "STA_ACTION_JOING_4WAY",/*STA_ACTION_JOING_4WAY*/
                        "STA_ACTION_LEAVING" /*STA_ACTION_LEAVING*/
                        };

    struct ST_SOC_STATUS{
        u8  u8SocState;
        u32 u32SocAction;
    }*ps1=NULL;

    ps1=(struct ST_SOC_STATUS *)data;
    //LOG_PRINTF("u8SocState=%d, u32SocAction=%d\r\n",ps1->u8SocState,ps1->u32SocAction);
    LOG_PRINTF("\n  >> soc status:%s\r\n",sta_status[ps1->u8SocState]);
    LOG_PRINTF("\n  >> soc action:%s\r\n",sta_action[ps1->u32SocAction]);
}




#if 0

void _soc_evt_get_wsid_tbl(u32 evt_id, void *data, s32 len)
{
    struct mac_wsid_entry_st    *wsid_entry;
    s32 i;

    ASSERT(len == sizeof(struct mac_wsid_entry_st)*4);
    wsid_entry = (struct mac_wsid_entry_st *)data;
    LOG_PRINTF("  >> WSID Table:\n      ");
    for(i=0; i<4; i++) {
        if (GET_WSID_INFO_VALID(wsid_entry) == 0) {
            LOG_PRINTF("[%d]: Invalid\n      ", i);
            continue;
        }
        LOG_PRINTF("[%d]: OP Mode: %d, QoS: %s, HT: %s\n      ",
            i, GET_WSID_INFO_OP_MODE(wsid_entry),
            ((GET_WSID_INFO_QOS_EN(wsid_entry)==0)? "disable": "enable"),
            ((GET_WSID_INFO_HT_MODE(wsid_entry)==0)? "disable": "enable")
        );
        LOG_PRINTF("      STA-MAC: %02x:%02x:%02x:%02x:%02x:%02x\n      ",
            wsid_entry->sta_mac.addr[0], wsid_entry->sta_mac.addr[1],
            wsid_entry->sta_mac.addr[2], wsid_entry->sta_mac.addr[3],
            wsid_entry->sta_mac.addr[4], wsid_entry->sta_mac.addr[5]
        );
    }
}


void _soc_evt_get_addba_req(u32 evt_id, void *data, s32 len)
{

	struct cfg_addba_resp *addba_resp;
    struct resp_evt_result *rx_addba_req = (struct resp_evt_result *)data;
	g_ba_session.policy=rx_addba_req->u.addba_req.policy;
	g_ba_session.tid=rx_addba_req->u.addba_req.tid;
	g_ba_session.buf_size=rx_addba_req->u.addba_req.agg_size;
	g_ba_session.timeout=rx_addba_req->u.addba_req.timeout;



    addba_resp = (void *)MALLOC (sizeof(struct cfg_addba_resp));
	addba_resp->dialog_token=1;//spec mention to set nonzero value
	addba_resp->policy=g_ba_session.policy;
	addba_resp->tid=g_ba_session.tid;
	addba_resp->buf_size=g_ba_session.buf_size;
	addba_resp->timeout=g_ba_session.timeout;
	addba_resp->status=0;
	addba_resp->start_seq_num=rx_addba_req->u.addba_req.start_seq_num;


    if (ssv6xxx_wifi_send_addba_resp(addba_resp) < 0)
       	LOG_PRINTF("Command failed !!\n");

    FREE(addba_resp);

}


void _soc_evt_get_delba(u32 evt_id, void *data, s32 len)
{
    struct resp_evt_result *rx_delba = (struct resp_evt_result *)data;
	LOG_PRINTF("RCV DELBA: reason:%d\n",rx_delba->u.delba_req.reason_code);
	ssv6xxx_memset((void *)&g_ba_session,0x00,sizeof(BA_SESSION));

}

#endif


/* ====================== End of Get Command Handlers ====================*/



//-------------------------------------------------------------------------------------
void cli_ps_t_handler(void* data1, void* data2);

extern void cmd_loop_pattern(void);

extern struct task_info_st g_cli_task_info[];
void _host_event_handler(u32 evt_id, void *data, s32 len)
{
    switch (evt_id) {
    case SOC_EVT_LOG:
        //LOG_PRINTF("SOC_EVT_LOG\n");
        //_soc_evt_handler_ssv6xxx_log(evt_id, data, len);
        break;
    case SOC_EVT_SCAN_RESULT:
        _scan_result_handler(data);
        break;
    case SOC_EVT_SCAN_DONE:
        _scan_down_handler(data);
        break;
    case SOC_EVT_SCONFIG_SCAN_DONE:
        _sconfig_scan_done(data);
        break;
    case SOC_EVT_GET_SOC_STATUS:
    	_soc_evt_get_soc_status(data);
        break;
    case SOC_EVT_BUS_LOOPBACK:
        _bus_loopback_handler(data);
        break;
    #ifdef USE_CMD_RESP
    case SOC_EVT_CMD_RESP:
        _handle_cmd_resp(evt_id, data, len);
        break;
    case SOC_EVT_DEAUTH:
        if(g_cli_ps_lp)
        {
            os_cancel_timer(cli_ps_t_handler,(void*)1,NULL);
            os_cancel_timer(cli_ps_t_handler,(void*)2,NULL);
        }
        _deauth_handler(data);
        break;
    #else // USE_CMD_RESP
    case SOC_EVT_JOIN_RESULT:
        _join_result_handler(data);
        break;
    case SOC_EVT_LEAVE_RESULT:
        g_cli_joining=false;
        if(g_cli_ps_lp)
        {
            os_cancel_timer(cli_ps_t_handler,(void*)1,NULL);
            os_cancel_timer(cli_ps_t_handler,(void*)2,NULL);
        }
        _leave_result_handler(data);
        break;
    case SOC_EVT_GET_REG_RESP:
        _get_soc_reg_response(evt_id, data, len);
        break;

    /*=================================================*/
    #endif // USE_CMD_RESP

    case SOC_EVT_POLL_STATION://sending arp request event
    case SOC_EVT_STA_STATUS://sending station add/remove event
        break;
    case SOC_EVT_PS_WAKENED:
        if(g_cli_ps_lp)
        {
            MsgEvent *msg_evt = NULL;
            msg_evt = msg_evt_alloc();
            if(NULL!=msg_evt)
            {
                msg_evt->MsgType = MEVT_HOST_CMD;
                msg_evt->MsgData = SOC_EVT_PS_WAKENED;
                msg_evt_post(CLI_MBX, msg_evt);
            }
            else
            {
                LOG_PRINTF("%s:msg alloc fail for SOC_EVT_PS_WAKENED\r\n",__FUNCTION__);
            }
        }
        break;
    case SOC_EVT_PS_SETUP_OK:
        break;
    default:
        LOG_PRINTF("Unknown host event received. %d\r\n", evt_id);
        break;
    }
} // end of - _host_event_handler -

extern OsSemaphore busLoopBackSem;
extern void *pLoopBackIn;
static void _bus_loopback_handler (void *data)
{
    ssv6xxx_memcpy((void *)OS_FRAME_GET_DATA(pLoopBackIn),data,OS_FRAME_GET_DATA_LEN(pLoopBackIn));
    LOG_PRINTF("Send sem\r\n");
    OS_SemSignal(busLoopBackSem);

    return;
}

#ifdef USE_CMD_RESP
void _handle_cmd_resp (u32 evt_id, u8 *data, s32 len)
{
	struct resp_evt_result *resp = (struct resp_evt_result *)data;

	if (resp->result != CMD_OK)
	{
		LOG_PRINTF("Command %d is not OK with code %d.\n", resp->cmd, resp->result);
		return;
    }
    switch (resp->cmd)
        {
        case SSV6XXX_HOST_CMD_SCAN:
			LOG_PRINTF("Scan done.\n");
            break;
        case SSV6XXX_HOST_CMD_JOIN:
            _join_result_handler(data);
            break;
        case SSV6XXX_HOST_CMD_LEAVE:
            _leave_result_handler(data);
            break;
        }

} // end of - _handle_cmd_resp -

void _deauth_handler (void *data)
{
    struct resp_evt_result *leave_res = (struct resp_evt_result *)data;
	LOG_PRINTF("Deauth from AP (reason=%d) !!\n", leave_res->u.leave.reason_code);
    netmgr_netif_link_set(LINK_DOWN);
}

#endif // USE_CMD_RESP

static void _scan_down_handler (void *data)
{
    struct resp_evt_result *scan_done = (struct resp_evt_result *)data;
    if(scan_done->u.scan_done.result_code==0){
        LOG_PRINTF("Scan Done\r\n");
    }else{
        LOG_PRINTF("Scan FAIL\r\n");
    }
    return;
}
static void _sconfig_scan_done (void *data)
{
    struct resp_evt_result *sconfig_done = (struct resp_evt_result *)data;
    u8 ssid_buf[MAX_SSID_LEN+1]={0};
    if(sconfig_done->u.sconfig_done.result_code==0){
        MEMCPY((void*)ssid_buf,(void*)sconfig_done->u.sconfig_done.ssid,sconfig_done->u.sconfig_done.ssid_len);
        LOG_PRINTF("SconfigDone. SSID:%s, PWD:%s , rand=%d\r\n",ssid_buf,sconfig_done->u.sconfig_done.pwd,sconfig_done->u.sconfig_done.rand);
    }else{
        LOG_PRINTF("Sconfig FAIL\r\n");
    }
    return;
}

void _scan_result_handler(void *data)
{
	s32	pairwise_cipher_index=0,group_cipher_index=0;
	u8		sec_str[][7]={"OPEN","WEP40","WEP104","TKIP","CCMP"};

    ap_info_state *scan_res = (ap_info_state*)data;

    char *act = NULL;
    char act_str[][7]={"Remove","Add","Modify"};
    u8 ssid_buf[MAX_SSID_LEN+1]={0};

    act = act_str[scan_res->act];
    LOG_DEBUGF(LOG_L2_STA, ("Action: %s AP Info:\r\n",act));
    LOG_DEBUGF(LOG_L2_STA, ("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
               scan_res->apInfo[scan_res->index].bssid.addr[0], scan_res->apInfo[scan_res->index].bssid.addr[1],
			   scan_res->apInfo[scan_res->index].bssid.addr[2], scan_res->apInfo[scan_res->index].bssid.addr[3],
			   scan_res->apInfo[scan_res->index].bssid.addr[4], scan_res->apInfo[scan_res->index].bssid.addr[5]));
    MEMCPY((void*)ssid_buf,(void*)scan_res->apInfo[scan_res->index].ssid.ssid,scan_res->apInfo[scan_res->index].ssid.ssid_len);
    LOG_DEBUGF(LOG_L2_STA, ("SSID: %s \t", ssid_buf));
    LOG_DEBUGF(LOG_L2_STA, ("@Channel Idx: %d\r\n", scan_res->apInfo[scan_res->index].channel_id));

    if(scan_res->apInfo[scan_res->index].capab_info&BIT(4))
    {
        LOG_DEBUGF(LOG_L2_STA, ("Secure Type=[%s]\r\n",
                   scan_res->apInfo[scan_res->index].proto&WPA_PROTO_WPA?"WPA":
                   scan_res->apInfo[scan_res->index].proto&WPA_PROTO_RSN?"WPA2":"WEP"));


        if(scan_res->apInfo[scan_res->index].pairwise_cipher[0])
        {
            pairwise_cipher_index=0;
            LOG_DEBUGF(LOG_L2_STA, ("Pairwise cipher="));

            for(pairwise_cipher_index=0;pairwise_cipher_index<8;pairwise_cipher_index++)
            {
                if(scan_res->apInfo[scan_res->index].pairwise_cipher[0]&BIT(pairwise_cipher_index))
                {
                    LOG_DEBUGF(LOG_L2_STA, ("[%s] ",sec_str[pairwise_cipher_index]));
                }
            }
            LOG_DEBUGF(LOG_L2_STA, ("\r\n"));
        }
        if(scan_res->apInfo[scan_res->index].group_cipher)
        {
            group_cipher_index=0;
            LOG_DEBUGF(LOG_L2_STA, ("Group cipher="));
            for(group_cipher_index=0;group_cipher_index<8;group_cipher_index++)
            {
                if(scan_res->apInfo[scan_res->index].group_cipher&BIT(group_cipher_index))
                {
                    LOG_DEBUGF(LOG_L2_STA, ("[%s] ",sec_str[group_cipher_index]));
                }
            }
            LOG_DEBUGF(LOG_L2_STA, ("\r\n"));
            }
        }
    else
    {
        LOG_DEBUGF(LOG_L2_STA, ("Secure Type=[NONE]\r\n"));
    }
    LOG_DEBUGF(LOG_L2_STA, ("RCPI=%d\r\n",scan_res->apInfo[scan_res->index].rxphypad.rpci));
    LOG_DEBUGF(LOG_L2_STA, ("\r\n"));
}

void _join_result_handler (void *data)
{
    struct resp_evt_result *join_res = (struct resp_evt_result *)data;
    if (join_res->u.join.status_code != 0)
    {
        LOG_PRINTF("Join failure!!\r\n");
		return;
    }

    LOG_PRINTF("Join success!!\r\n");
    LOG_DEBUGF(LOG_L2_STA, ("Join AID=%d\r\n",join_res->u.join.aid));
    
    g_cli_joining=true;
	//ssv6xxx_wifi_apply_security();
    //netmgr_netif_link_set(LINK_UP);

} // end of - _join_result_handler -


void _leave_result_handler (void *data)
{
    struct resp_evt_result *leave_res = (struct resp_evt_result *)data;
    LOG_PRINTF("Leave received deauth from AP!!\r\n");
    LOG_DEBUGF(LOG_L2_STA, ("Reason Code=%d\r\n",leave_res->u.leave.reason_code));
    //netmgr_netif_link_set(LINK_DOWN);
}

void _get_soc_reg_response(u32 eid, void *data, s32 len)
{
    LOG_PRINTF("%s(): HOST_EVENT=%d: len=%d\n", __FUNCTION__, eid, len);
//    memcpy((void *)g_soc_cmd_rx_buffer, (void *)data, len);
//    g_soc_cmd_rx_ready = 1;
}

extern void cmd_ifconfig(s32 argc, char *argv[]);
void cli_ps_resume(void)
{
    s32 ret;

    LOG_PRINTF("g_cli_ps_lp=%d\r\n",g_cli_ps_lp);
    cmd_ifconfig(1, NULL);
    if(g_cli_ps_lp > 0)
    {
        ret = os_create_timer(3000,cli_ps_t_handler,(void*)1,NULL, (void*)CLI_MBX); // sleep after 3 sec
        if(ret==OS_FAILED)
            LOG_PRINTF("creat ps 1 tmr fail\r\n");
            
    }
    
    if(g_cli_ps_lp == 2)
    {
        ret = os_create_timer(8000,cli_ps_t_handler,(void*)2,NULL, (void*)CLI_MBX); // wakeup after 8 sec
        if(ret==OS_FAILED)
            LOG_PRINTF("creat ps 2 tmr fail\r\n");
    }
}
void _cmd_wifi_ps(void)
{
    ipinfo info;
    struct cfg_ps_request wowreq;
    netmgr_ipinfo_get(WLAN_IFNAME, &info);
    MEMSET((void*)&wowreq,0,sizeof(wowreq));
    LOG_PRINTF("ipv4=%x\r\n",info.ipv4);
    wowreq.ipv4addr = info.ipv4;
    wowreq.dtim_multiple = 3;
    wowreq.host_ps_st = HOST_PS_SETUP;
    ssv6xxx_wifi_pwr_saving(&wowreq,TRUE);
}

void _cmd_wifi_wkp(char *argv[])
{
    ssv6xxx_wifi_wakeup();
}
void cli_ps_t_handler(void* data1, void* data2)
{
    u32 val = (u32)data1;
    LOG_PRINTF("%s val=%d\r\n",__func__,val);
    switch (val)
    {
        case 1:
            _cmd_wifi_ps();    
            break;
        case 2:            
            _cmd_wifi_wkp(NULL);
            break;
    }
    
}

void cmd_iw(s32 argc, char *argv[])
{
//    ssv6xxx_wifi_reg_evt_cb(_host_event_handler);
//gHCmdEngInfo

	if (argc<2)
		return;

    if (strcmp(argv[1], "scan")==0) {
        if (argc >= 3)
		    _cmd_wifi_scan(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
#if(ENABLE_SMART_CONFIG==1)
	}else if (strcmp(argv[1], "sconfig")==0) {
        if (argc >= 4)
		    _cmd_wifi_sconfig(argc - 2, &argv[2]);
        else{
            _sconfig_usage();
        }
#endif
	} else if (strcmp(argv[1], "join")==0) {
        if (argc >= 3)
            _cmd_wifi_join(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    }  else if (strcmp(argv[1], "join-other")==0) {
        if (argc >= 3)
            _cmd_wifi_join_other(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    } else if (strcmp(argv[1], "leave")==0) {
        if (argc==2)
            _cmd_wifi_leave(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    } else if (strcmp(argv[1], "list")==0) {
        if (argc == 2)
        {
            _cmd_wifi_list(argc - 2, &argv[2]);
        }
        else
            LOG_PRINTF("Invalid arguments.\n");
        /*
    } else if (strcmp(argv[1], "ap")==0) {
        if (argc >= 3)
            _cmd_wifi_ap(argc - 2, &argv[2]);
		else
            LOG_PRINTF("Invalid arguments.\n");
        */
    } else if (strcmp(argv[1], "ps")==0) {
        g_cli_ps_lp = 0;
        if (strcmp(argv[2], "lp")==0)
        {    
            g_cli_ps_lp = 1;
        }
        _cmd_wifi_ps();
        if (strcmp(argv[2], "wk")==0)
        {
            g_cli_ps_lp = 2;
            os_create_timer(5000,cli_ps_t_handler,(void*)g_cli_ps_lp,NULL, (void*)CLI_MBX);
        }
        
    }else if (strcmp(argv[1], "wkp")==0) {
        g_cli_ps_lp = 0;
        os_cancel_timer(cli_ps_t_handler,(void*)1,NULL);
        os_cancel_timer(cli_ps_t_handler,(void*)2,NULL);
        _cmd_wifi_wkp(&argv[2]);
    }else if (strcmp(argv[1], "txduty")==0) {
        ssv6xxx_wifi_set_tx_duty((u32)ssv6xxx_atoi(argv[2]),(u32)ssv6xxx_atoi(argv[3]));
    }else if (strcmp(argv[1], "src")==0) {
        ssv6xxx_set_TXQ_SRC_limit((u32)ssv6xxx_atoi(argv[2]),(u32)ssv6xxx_atoi(argv[3]));
    }    
	else {
        LOG_PRINTF("Invalid iw command.\n");
    }
} // end of - cmd_iw -

#if(ENABLE_SMART_CONFIG==1)
extern u16 g_SconfigChannelMask;
extern u32 g_sconfig_solution;
void _sconfig_usage(void)
{
    LOG_PRINTF("Usage:\r\n");
    LOG_PRINTF("      iw sconfig [solution] [channel mask]\r\n");
	LOG_PRINTF("      solution name = [SLINK|AIRKISS]\r\n");
}
void _cmd_wifi_sconfig (s32 argc, char *argv[])
{
    /**
     *  sconfig Command Usage:
     *  iw sconfig <chan_mask>
     */
    u16 channel = (u16)strtol(argv[1],NULL,16);
    if(0==STRCMP(argv[0],"AIRKISS")){
        g_sconfig_solution=WECHAT_AIRKISS_IN_FW;
    }else if(0==STRCMP(argv[0],"SLINK")){
        g_SconfigChannelMask=channel; //This variable is only for RD use, customers don't need to modify it.
        g_sconfig_solution=ICOMM_SMART_LINK;
    }else{
        _sconfig_usage();
        return;
    }
    netmgr_wifi_sconfig_async(channel);
} // end of - _cmd_wifi_sconfig -

#endif
void _cmd_wifi_scan (s32 argc, char *argv[])
{
    /**
     *  Scan Command Usage:
     *  iw scan <chan_mask> <ssid0> <ssid1> <ssid2> ...
     */
    int num_ssids = argc - 1;
    u16 channel = (u16)strtol(argv[0],NULL,16);

    if (num_ssids > 0)
    {
        netmgr_wifi_scan_async(channel, &(argv[1]), num_ssids);
    }
    else
    {
        netmgr_wifi_scan_async(channel, NULL, 0);
    }
} // end of - _cmd_wifi_scan -


//iw join ap_name [wep|wpa|wpa2] passwd
void _cmd_wifi_join (s32 argc, char *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw join ssid
     */
	int ret = -1;
    wifi_sta_join_cfg *join_cfg = NULL;

    join_cfg = (wifi_sta_join_cfg *)MALLOC(sizeof(wifi_sta_join_cfg));
    if(NULL==join_cfg)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }

    if (argc > 0)
    {
        STRCPY((char *)join_cfg->ssid.ssid, argv[0]);
        join_cfg->ssid.ssid_len=STRLEN(argv[0]);
        if (argc == 2)
        {
            STRCPY((char *)join_cfg->password, argv[1]);
        }
    }
    else
    {
        FREE(join_cfg);
        return;
    }

    ret = netmgr_wifi_join_async(join_cfg);
    if (ret != 0)
    {
	    LOG_PRINTF("netmgr_wifi_join_async failed !!\r\n");
    }

    FREE(join_cfg);
} // end of - _cmd_wifi_join -

void _cmd_wifi_join_other (s32 argc, char *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw join ssid
     */
	int ret = -1;
    wifi_sta_join_cfg *join_cfg = NULL;

    join_cfg = (wifi_sta_join_cfg *)MALLOC(sizeof(wifi_sta_join_cfg));
    if(NULL==join_cfg)
    {
        LOG_PRINTF("%s(%d):malloc fail\r\n",__FUNCTION__,__LINE__);
        return;
    }

    if (argc > 0)
    {
        STRCPY((char *)join_cfg->ssid.ssid, argv[0]);
        join_cfg->ssid.ssid_len=STRLEN(argv[0]);
        if (argc == 2)
        {
            STRCPY((char *)join_cfg->password, argv[1]);
        }
    }
    else
    {
        FREE(join_cfg);
        return;
    }

    ret = netmgr_wifi_join_other_async(join_cfg);
    if (ret != 0)
    {
	    LOG_PRINTF("netmgr_wifi_join_other_async failed !!\r\n");
    }

    FREE(join_cfg);
} // end of - _cmd_wifi_join -

void _cmd_wifi_leave(s32 argc, char *argv[])
{
    /**
	 *	Leave Command Usage:
	 *	host leave ... ...
	 */
	int ret = -1;

    ret = netmgr_wifi_leave_async();
    if (ret != 0)
    {
	    LOG_PRINTF("netmgr_wifi_leave failed !!\r\n");
    }
} // end of - _cmd_wifi_leave -

void _cmd_wifi_list(s32 argc, char *argv[])
{
    u32 i=0,AP_cnt;
    s32     pairwise_cipher_index=0,group_cipher_index=0;
    u8      sec_str[][7]={"OPEN","WEP40","WEP104","TKIP","CCMP"};
    u8  ssid_buf[MAX_SSID_LEN+1]={0};
    Ap_sta_status connected_info;

    struct ssv6xxx_ieee80211_bss *ap_list = NULL;
    AP_cnt = ssv6xxx_get_aplist_info((void *)&ap_list);

    
    MEMSET(&connected_info , 0 , sizeof(Ap_sta_status));
    ssv6xxx_wifi_status(&connected_info);

    if((ap_list==NULL) || (AP_cnt==0))
    {
        LOG_PRINTF("AP list empty!\r\n");
        return;
    }
    for (i=0; i<AP_cnt; i++)
    {

        if(ap_list[i].channel_id!= 0)
		{
		    LOG_PRINTF("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            ap_list[i].bssid.addr[0],  ap_list[i].bssid.addr[1], ap_list[i].bssid.addr[2],  ap_list[i].bssid.addr[3],  ap_list[i].bssid.addr[4],  ap_list[i].bssid.addr[5]);
            MEMSET((void*)ssid_buf,0,sizeof(ssid_buf));
            MEMCPY((void*)ssid_buf,(void*)ap_list[i].ssid.ssid,ap_list[i].ssid.ssid_len);
            LOG_PRINTF("SSID: %s\t", ssid_buf);
			LOG_PRINTF("@Channel Idx: %d\r\n", ap_list[i].channel_id);
            if(ap_list[i].capab_info&BIT(4)){
                LOG_PRINTF("Secure Type=[%s]\r\n",
                ap_list[i].proto&WPA_PROTO_WPA?"WPA":
                ap_list[i].proto&WPA_PROTO_RSN?"WPA2":"WEP");

                if(ap_list[i].pairwise_cipher[0]){
                    pairwise_cipher_index=0;
                    LOG_PRINTF("Pairwise cipher=");
                    for(pairwise_cipher_index=0;pairwise_cipher_index<8;pairwise_cipher_index++){
                        if(ap_list[i].pairwise_cipher[0]&BIT(pairwise_cipher_index)){
                            LOG_PRINTF("[%s] ",sec_str[pairwise_cipher_index]);
                        }
                    }
                    LOG_PRINTF("\r\n");
                }
                if(ap_list[i].group_cipher){
                    group_cipher_index=0;
                    LOG_PRINTF("Group cipher=");
                    for(group_cipher_index=0;group_cipher_index<8;group_cipher_index++){
                        if(ap_list[i].group_cipher&BIT(group_cipher_index)){
                            LOG_PRINTF("[%s] ",sec_str[group_cipher_index]);
                        }
                    }
                    LOG_PRINTF("\r\n");
                }
            }else{
                LOG_PRINTF("Secure Type=[NONE]\r\n");
            }
          
            if(!memcmp((void *)ap_list[i].bssid.addr,(void *)connected_info.u.station.apinfo.Mac,ETHER_ADDR_LEN)){
                LOG_PRINTF("RSSI=-%d (dBm)\r\n",ssv6xxx_get_rssi_by_mac((u8 *)ap_list[i].bssid.addr));
            }
            else{
                LOG_PRINTF("RSSI=-%d (dBm)\r\n",ap_list[i].rxphypad.rpci);
            }
            LOG_PRINTF("\r\n");
		}

    }
    FREE((void *)ap_list);
} // end of - _cmd_wifi_list -




// iw ap ssid sectype password
// iw ap ssv  password
void _cmd_wifi_ap (s32 argc, char *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw ap ssid
     */
    const char *sec_name;
    u8 ssid_buf[MAX_SSID_LEN+1]={0};
	struct cfg_set_ap_cfg ApCfg;
    MEMSET(&ApCfg, 0, sizeof(struct cfg_set_ap_cfg));

//Fill SSID
    ApCfg.ssid.ssid_len = strlen(argv[0]);
    MEMCPY( (void*)&ApCfg.ssid.ssid, (void*)argv[0], strlen(argv[0]));

//Fill PASSWD
	if (argc == 3)
		MEMCPY(&ApCfg.password, argv[2], strlen(argv[2]));


    if (argc == 1){
        sec_name = "open";
        ApCfg.sec_type = SSV6XXX_SEC_NONE;
    }
	else if (strcmp(argv[1], "wep40") == 0){
        sec_name = "wep40";
    	ApCfg.sec_type = SSV6XXX_SEC_WEP_40;
		if (argc != 3)
		{
	        ApCfg.password[0]= 0x31;
			ApCfg.password[1]= 0x32;
    		ApCfg.password[2]= 0x33;
	    	ApCfg.password[3]= 0x34;
			ApCfg.password[4]= 0x35;
    		ApCfg.password[5]= '\0';
		}


	}
	else if (strcmp(argv[1], "wep104") == 0){
			sec_name = "wep104";

			ApCfg.sec_type = SSV6XXX_SEC_WEP_104;
			if (argc != 3)
			{
				ApCfg.password[0]= '0';
				ApCfg.password[1]= '1';
				ApCfg.password[2]= '2';
				ApCfg.password[3]= '3';
				ApCfg.password[4]= '4';
				ApCfg.password[5]= '5';
				ApCfg.password[6]= '6';
				ApCfg.password[7]= '7';
				ApCfg.password[8]= '8';
				ApCfg.password[9]= '9';
				ApCfg.password[10]= '0';
				ApCfg.password[11]= '1';
				ApCfg.password[12]= '2';
				ApCfg.password[13]= '\0';
			}
    }
#ifndef CONFIG_NO_WPA2
	else if (strcmp(argv[1], "wpa2") == 0){
        sec_name = "wpa2";
	  	ApCfg.sec_type = SSV6XXX_SEC_WPA2_PSK;

		if (argc != 3)
		{
	        ApCfg.password[0]= 's';
		    ApCfg.password[1]= 'e';
	       	ApCfg.password[2]= 'c';
		    ApCfg.password[3]= 'r';
			ApCfg.password[4]= 'e';
			ApCfg.password[5]= 't';
	        ApCfg.password[6]= '0';
		    ApCfg.password[7]= '0';
			ApCfg.password[8]= '\0';
		}

    }
#endif
	else{
        LOG_PRINTF("ERROR: unkown security type: %s\n", argv[1]);
        sec_name = "open";
        //ApCfg.auth_alg = WPA_AUTH_ALG_OPEN;
        ApCfg.sec_type = SSV6XXX_SEC_NONE;
    }

    MEMCPY((void*)ssid_buf,(void*)ApCfg.ssid.ssid,ApCfg.ssid.ssid_len);
    LOG_PRINTF("AP configuration==>\nSSID:\"%s\" \nSEC Type:\"%s\" \nPASSWD:\"%s\"\n",
		ssid_buf, sec_name, ApCfg.password);



    if (ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_SET_AP_CFG, &ApCfg, sizeof(ApCfg)) < 0)
	    LOG_PRINTF("Command failed !!\n");


} // end of - _cmd_wifi_join -




void ssv6xxx_wifi_cfg(void)
{
    ssv6xxx_wifi_reg_evt_cb(_host_event_handler);
#ifndef CONFIG_NO_WPA2
#if (AP_MODE_ENABLE == 1)
    ssv6xxx_eapol_data_reg_cb();
#endif
#endif
}

void cmd_ctl(s32 argc, char *argv[])
{

    bool errormsg = FALSE;
    char *err_str = "";
    u8 ssid_buf[MAX_SSID_LEN+1]={0};

    if (argc <= 1)
    {
        errormsg = TRUE;
    }
	else if (strcmp(argv[1], "status")==0)
    {
        Ap_sta_status info;
        MEMSET(&info , 0 , sizeof(Ap_sta_status));
        errormsg = FALSE;
        ssv6xxx_wifi_status(&info);
        if(info.status)
            LOG_PRINTF("status:ON\r\n");
        else
            LOG_PRINTF("status:OFF\r\n");
        if((SSV6XXX_HWM_STA==info.operate)||(SSV6XXX_HWM_SCONFIG==info.operate))
        {
            LOG_PRINTF("Mode:%s, %s\r\n",(SSV6XXX_HWM_STA==info.operate)?"Station":"Sconfig",(info.u.station.apinfo.status == CONNECT) ? "connected" :"disconnected");
            LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                info.u.station.selfmac[0],
                info.u.station.selfmac[1],
                info.u.station.selfmac[2],
                info.u.station.selfmac[3],
                info.u.station.selfmac[4],
                info.u.station.selfmac[5]);
            MEMCPY((void*)ssid_buf,(void*)info.u.station.ssid.ssid,info.u.station.ssid.ssid_len);
            LOG_PRINTF("SSID:%s\r\n",ssid_buf);
            LOG_PRINTF("channel:%d\r\n",info.u.station.channel);
            LOG_PRINTF("AP Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                info.u.station.apinfo.Mac[0],
                info.u.station.apinfo.Mac[1],
                info.u.station.apinfo.Mac[2],
                info.u.station.apinfo.Mac[3],
                info.u.station.apinfo.Mac[4],
                info.u.station.apinfo.Mac[5]);
            if(is_valid_ether_addr(info.u.station.apinfo.Mac)){
                LOG_PRINTF("RSSI = -%d (dBm)\r\n",ssv6xxx_get_rssi_by_mac(info.u.station.apinfo.Mac));
            }
            else{
                LOG_PRINTF("RSSI = 0 (dBm)\r\n");
            }
        }
#if (AP_MODE_ENABLE == 1)        
        else if(SSV6XXX_HWM_AP==info.operate)
        {
            u32 statemp;
            u8 idx;
            struct apmode_sta_info *sta_info=NULL;
    
            sta_info = (void*)OS_MemAlloc(sizeof(struct apmode_sta_info));
            if(!sta_info)
            {
                LOG_PRINTF("malloc sta_info fail \r\n");
                return ;
            }
            
            LOG_PRINTF("Mode:AP\r\n");
            LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                info.u.ap.selfmac[0],
                info.u.ap.selfmac[1],
                info.u.ap.selfmac[2],
                info.u.ap.selfmac[3],
                info.u.ap.selfmac[4],
                info.u.ap.selfmac[5]);
            MEMCPY((void*)ssid_buf,(void*)info.u.ap.ssid.ssid,info.u.ap.ssid.ssid_len);
            LOG_PRINTF("SSID:%s\r\n",ssid_buf);
            LOG_PRINTF("channel:%d\r\n",info.u.ap.channel);
            LOG_PRINTF("Station number:%d\r\n",info.u.ap.stanum);
            for(statemp=0; statemp < info.u.ap.stanum ;statemp ++ )
            {
                LOG_PRINTF("station Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                    info.u.ap.stainfo[statemp].Mac[0],
                    info.u.ap.stainfo[statemp].Mac[1],
                    info.u.ap.stainfo[statemp].Mac[2],
                    info.u.ap.stainfo[statemp].Mac[3],
                    info.u.ap.stainfo[statemp].Mac[4],
                    info.u.ap.stainfo[statemp].Mac[5]);
            }

            for(idx=0;idx<WLAN_MAX_STA;idx++)
            {
                MEMSET(sta_info , 0 , sizeof(struct apmode_sta_info));

                LOG_PRINTF("==================STA:%d=================\r\n",idx);
                APStaInfo_PrintStaInfo(idx);
                
                if(ssv6xxx_get_sta_info_by_aid(sta_info,idx)==SSV6XXX_SUCCESS)
                {
                    if(is_valid_ether_addr(sta_info->addr))
                    {
                        if(sta_info->arp_retry_count>STA_TIMEOUT_RETRY_COUNT)
                        {
                            LOG_PRINTF("Inactive\r\n");
                        }
                        else
                        {
                            LOG_PRINTF("Active\r\n");
                        }
                    
                        LOG_PRINTF("addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                            sta_info->addr[0],
                            sta_info->addr[1],
                            sta_info->addr[2],
                            sta_info->addr[3],
                            sta_info->addr[4],
                            sta_info->addr[5]);
                    
                        LOG_PRINTF("RSSI = -%d (dBm) \r\n", sta_info->rcpi);
                   }
                }
                LOG_PRINTF("========================================\r\n");
            }
            OS_MemFree(sta_info);
      
            
         }
#endif        
    }
#if (AP_MODE_ENABLE == 1)    
    else if (strcmp(argv[1], "ap")==0&&argc >= 3)
    {
        Ap_setting ap;
        MEMSET(&ap , 0 , sizeof(Ap_setting));
        ap.channel =EN_CHANNEL_AUTO_SELECT;
        //instruction dispatch
        // ctl ap on [ap_name] [channel] [security] [password]
        switch(argc)
        {
            #if 0
            case 3: // wifi ap off
                if(strcmp(argv[2], "off")==0)
                {
                    errormsg =FALSE;
                    ap.status = FALSE;
                }

                break;
            #endif
            case 4: // only ssid , security open , //ctl ap on [ap_name]
                if((strcmp(argv[2], "on" )== 0)&&(strlen(argv[3])<=MAX_SSID_LEN))
                {
                    errormsg =FALSE;
                    ap.status = TRUE;
                    ap.security = SSV6XXX_SEC_NONE;
                    ap.ssid.ssid_len = strlen(argv[3]);
                    MEMCPY( (void*)ap.ssid.ssid, (void*)argv[3], strlen(argv[3]));
                }
                else
                {
                    errormsg = TRUE;
                    err_str = "SSID is too long";
                }
                break;

            case 5: // only ssid , security open, set channel, // ctl ap on [ap_name] [channel]
                    if((strcmp(argv[2], "on" )== 0)&&(0<strtol(argv[4],NULL,10))&&
                        (strtol(argv[4],NULL,10)<=14)&&(strlen(argv[3])<=MAX_SSID_LEN))
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = SSV6XXX_SEC_NONE;
                        ap.ssid.ssid_len = strlen(argv[3]);
                        ap.channel = (u8)strtol(argv[4],NULL,10);
                        MEMCPY( (void*)ap.ssid.ssid, (void*)argv[3], strlen(argv[3]));
                    }
                    else
                    {
                        errormsg = TRUE;
                        err_str = "Channel invalid/ SSID is too long";
                    }
                    break;


            case 6: //have security type // ctl ap on [ap_name] [security] [password]
                if(strcmp(argv[2], "on") == 0&&(strlen(argv[3])<=MAX_SSID_LEN))
                {
                    if(strcmp(argv[4], "wep") == 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        if(strlen(argv[5]) == 5)
                            ap.security = 	SSV6XXX_SEC_WEP_40;
                        else if (strlen(argv[5]) == 13)
                            ap.security = 	SSV6XXX_SEC_WEP_104;
                        else
                        {
                            LOG_PRINTF("WEP key length must be 5 or 13 character. \r\n");
                            errormsg =TRUE;
                            break;
                        }
                    }
#ifndef CONFIG_NO_WPA2
					else if(strcmp(argv[4], "wpa2") == 0)
				    {
				    	errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = SSV6XXX_SEC_WPA2_PSK;
						ap.proto = WPA_PROTO_RSN;
                        ap.key_mgmt = WPA_KEY_MGMT_PSK ;
                        ap.group_cipher=WPA_CIPHER_CCMP;
                        ap.pairwise_cipher = WPA_CIPHER_CCMP;
					    if((strlen(argv[5]) <8) || (strlen(argv[5])>63))

                        {
                            LOG_PRINTF("WEP key length must be 8~63 character. \r\n");
                            errormsg =TRUE;
                            break;
                        }

				    }
#endif
                    else
                    {
                        LOG_PRINTF("SSID:%s, Security type:%s, Password:%s. \r\n",argv[3],argv[4],argv[5] );
                        errormsg =TRUE;
                        break;

                    }
                    STRCPY((char *)ap.password, argv[5]);
                    ap.ssid.ssid_len = strlen(argv[3]);
                    MEMCPY( (void*)ap.ssid.ssid, (void*)argv[3], strlen(argv[3]));

                }
                break;
            case 7: //have security type and channel setting // ctl ap on [ap_name] [channel] [security] [password]
                if(strcmp(argv[2], "on") == 0&&(0<strtol(argv[4],NULL,10))&&
                        (strtol(argv[4],NULL,10)<=14)&&(strlen(argv[3])<=MAX_SSID_LEN))
                {
                    if(strcmp(argv[5], "wep") == 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        if(strlen(argv[6]) == 5)
                            ap.security = 	SSV6XXX_SEC_WEP_40;
                        else if (strlen(argv[6]) == 13)
                            ap.security = 	SSV6XXX_SEC_WEP_104;
                        else
                        {
                            LOG_PRINTF("WEP key length must be 5 or 13 character. \r\n");
                            errormsg =TRUE;
                        }
                    }
#ifndef CONFIG_NO_WPA2
					else if(strcmp(argv[5], "wpa2") == 0)
				    {
				    	errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = SSV6XXX_SEC_WPA2_PSK;
						ap.proto = WPA_PROTO_RSN;
                        ap.key_mgmt = WPA_KEY_MGMT_PSK ;
                        ap.group_cipher=WPA_CIPHER_CCMP;
                        ap.pairwise_cipher = WPA_CIPHER_CCMP;
						LOG_PRINTF("SSID:%s, channel:%d, Security type:%s, Password:%s. \r\n",argv[3],strtol(argv[4],NULL,10),argv[5],argv[6] );
						LOG_PRINTF("ap.pairwise_cipher=%d+++++++\n\r",ap.pairwise_cipher);
					    if((strlen(argv[6]) <8) || (strlen(argv[6])>63))

                        {
                            LOG_PRINTF("s key length must be 8~63 character. \r\n");
                            errormsg =TRUE;
                            break;
                        }

				    }
#endif
                    else
                    {
                        LOG_PRINTF("SSID:%s, channel:%d, Security type:%s, Password:%s. \r\n",argv[3],strtol(argv[4],NULL,10),argv[5],argv[6] );
                        errormsg =TRUE;
                        break;

                    }
                    STRCPY((char *)ap.password, argv[6]);
                    ap.ssid.ssid_len = strlen(argv[3]);
                    MEMCPY( (void*)ap.ssid.ssid, (void*)argv[3], strlen(argv[3]));
                    ap.channel = (u8)strtol(argv[4],NULL,10);

                }
                break;
            default:
                errormsg = TRUE;
                break;


        }
        if(!errormsg)
        {
	        if (netmgr_wifi_control_async(SSV6XXX_HWM_AP, &ap, NULL) == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;
        }
        else
        {
            LOG_PRINTF("Invalid wifictl command. %s\r\n",err_str);
        }


	}
#endif    
    else if (strcmp(argv[1], "sta")==0&&argc >= 3)
	{
        Sta_setting sta;
        MEMSET(&sta, 0 , sizeof(Sta_setting));

        if(strcmp(argv[2], "on") == 0)
        {
            sta.status = TRUE;
        }
        else
        {
            #if 0
            sta.status = FALSE;
            #endif
            errormsg = TRUE;
        }

        if (!errormsg)
        {
            if (netmgr_wifi_control_async(SSV6XXX_HWM_STA, NULL, &sta) == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;
        }


    }
#if(ENABLE_SMART_CONFIG==1)
    else if (strcmp(argv[1], "sconfig")==0&&argc >= 3)
	{
        Sta_setting sta;
        MEMSET(&sta, 0 , sizeof(Sta_setting));

        if(strcmp(argv[2], "on") == 0)
        {
            sta.status = TRUE;
        }
        else
        {
            #if 0
            sta.status = FALSE;
            #endif
            errormsg = TRUE;
        }

        if (!errormsg)
        {
            if (netmgr_wifi_control_async(SSV6XXX_HWM_SCONFIG, NULL, &sta) == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;
        }

    }
#endif
    else if (strcmp(argv[1], "off")==0)
	{
        Sta_setting sta;
        MEMSET(&sta, 0 , sizeof(Sta_setting));
        sta.status = FALSE;

        if (!errormsg)
        {
            if (netmgr_wifi_control_async(SSV6XXX_HWM_STA, NULL, &sta) == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else
                errormsg = FALSE;
        }
    }
    else if (strcmp(argv[1], "dt")==0)
    {
		ssv6xxx_drv_detect_card();
    }else{
        LOG_PRINTF("Invalid wifictl command.\r\n");
    }

	if(FALSE==errormsg)
    {
        LOG_PRINTF("OK.\r\n");
    }
    else
    {
        char * cmd_name = "ctl";
        show_invalid_para(cmd_name);
    }
} // end of - cmd_iw -


void cmd_rc(s32 argc, char *argv[])
{

    if (strcmp(argv[1], "mask")==0&&argc == 3)
    {
        u32 rate_mask = (u16)ssv6xxx_atoi(argv[2]);

        ssv6xxx_set_rc_value(RC_RATEMASK, rate_mask);
        return;
    }
    else if (strcmp(argv[1], "preprbfrm")==0&&argc == 3)
    {
        u32 param = (u32)ssv6xxx_atoi(argv[2]);

        if ((param == 0) || (param == 1))
        {
            ssv6xxx_set_rc_value(RC_PREPRBFRM, param);
            return;
        }                        
    }
    else if (strcmp(argv[1], "upperfastestb")==0&&argc == 3)
    {
        u32 param = (u32)ssv6xxx_atoi(argv[2]);
        
        ssv6xxx_set_rc_value(RC_UPPERFASTESTB, param);        
        return;
    }
    else if (strcmp(argv[1], "resent")==0&&argc == 3)
    {
        u32 param = (u32)ssv6xxx_atoi(argv[2]);

        if ((param == 0) || (param == 1))
        {
            ssv6xxx_set_rc_value(RC_RESENT, param);        
            LOG_PRINTF("resent = %d!!\r\n",param); 
            return;
        }
    }
    else if (strcmp(argv[1], "per")==0&&argc == 4)
    {
        u16 up = (u16)ssv6xxx_atoi(argv[2]);
        u16 down = (u16)ssv6xxx_atoi(argv[3]);

        u32 param = ((up<<16)|down);
        ssv6xxx_set_rc_value(RC_PER, param);
        LOG_PRINTF("per %d/%d!!\r\n",(u32)ssv6xxx_atoi(argv[2]),(u32)ssv6xxx_atoi(argv[3]));            
        return;
    }
    else if(strcmp(argv[1], "direct")==0&&argc == 3)
    {
        ssv6xxx_set_rc_value(DIRECT_RATE_DW, (u32)ssv6xxx_atoi(argv[2]));
        LOG_PRINTF("direct =%d!!\r\n",(u32)ssv6xxx_atoi(argv[2]));
        return;
    }
    else if(strcmp(argv[1], "rtscts")==0&&argc == 3)
    {
        ssv6xxx_set_rc_value(FORCE_RTSCTS, (u32)ssv6xxx_atoi(argv[2]));
        LOG_PRINTF("rtscts =%d!!\r\n",(u32)ssv6xxx_atoi(argv[2]));
        return;
    }
    else if(strcmp(argv[1], "drateendian")==0&&argc == 3)
    {
        ssv6xxx_set_rc_value(RC_DRATE_ENDIAN, (u32)ssv6xxx_atoi(argv[2]));
        LOG_PRINTF("drateendian =%d!!\r\n",(u32)ssv6xxx_atoi(argv[2]));
        return;
    }
    LOG_PRINTF("Invalid wifictl command.\r\n");
    LOG_PRINTF("rc mask 0x0[FFF]\r\n");
    LOG_PRINTF("rc resent [0-1]\r\n");
    LOG_PRINTF("rc upperfastestb [1-50]\r\n");
    LOG_PRINTF("rc preprbfrm [0-1]\r\n");
    LOG_PRINTF("rc per [1-50] [1-50]\r\n");    
    LOG_PRINTF("rc drateendian [0-1]\r\n");        
    return;
}
void cmd_mib(s32 argc, char *argv[])
{

    extern s32 ssv6xxx_wifi_get_recover_cnt(void);
    extern s32 ssv6xxx_wifi_get_fw_interrupt_cnt(void);
    LOG_PRINTF("Recover count:%d\r\n",ssv6xxx_wifi_get_recover_cnt());
    LOG_PRINTF("Fw timer interrupt count:%u\r\n",ssv6xxx_wifi_get_fw_interrupt_cnt());

}

char *bmode[] = {"1M", "2M", "5.5M", "11M"};
void cmd_ampdu(s32 argc, char *argv[])
{
    if (argc == 3)
    {
        u8 mode = 0;
        u8 value = (u8)ssv6xxx_atoi(argv[2]);
        if ((strcmp(argv[1], "tx")==0) && ((value == 0) || (value == 1)))
        {
            mode = AMPDU_TX_OPT_ENABLE;
            LOG_PRINTF("AMPDU tx %s\r\n", (value == 0)?"off":"on");
        }
        else if ((strcmp(argv[1], "lastretryb")==0) && ((value == 0) || (value == 1)))
        {
            mode = AMPDU_TX_OPT_SET_LAST_TRY_BMODE;
            LOG_PRINTF("AMPDU last retry b mode is %s\r\n", (value == 0)?"disable":"enable");
        }
        else if ((strcmp(argv[1], "lastbrate")==0) && (value < 4))
        {
            mode = AMPDU_TX_OPT_SET_LAST_BMODE_RATE;
            LOG_PRINTF("AMPDU last retry b rate is %s\r\n", bmode[value]);
        }
        else if ((strcmp(argv[1], "lastbonce")==0) && ((value == 0) || (value == 1)))
        {
            mode = AMPDU_TX_OPT_SET_LAST_BMODE_RETRY;
            LOG_PRINTF("AMPDU last retry b retry once = %d\r\n", value);
        }
        else if ((strcmp(argv[1], "sessionchk")==0) && ((value == 0) || (value == 1)))
        {
            mode = AMPDU_TX_OPT_SET_BLOCK_NON_NMODE;
            LOG_PRINTF("AMPDU session check is %s\r\n", (value == 0)?"disable":"enable");
        }
        else if (strcmp(argv[1], "maxretry")==0)
        {
            mode = AMPDU_TX_OPT_SET_RETRY_MAX;
            LOG_PRINTF("AMPDU maxretry = %d\r\n", value);
        }
        else if (strcmp(argv[1], "maintry")==0)
        {
            mode = AMPDU_TX_OPT_SET_MAIN_TRY;
            LOG_PRINTF("AMPDU main try = %d\r\n", value);
        }
        else if (strcmp(argv[1], "rx")==0)
        {
            mode = AMPDU_RX_OPT_ENABLE;
            LOG_PRINTF("AMPDU rx = %d\r\n", value);
        }
        else if (strcmp(argv[1], "rxbufsize")==0)
        {
            mode = AMPDU_RX_OPT_BUF_SIZE;
            LOG_PRINTF("AMPDU rxbufsize = %d\r\n", value);
        }
        else
        {
            goto USAGE;
        }
        ssv6xxx_set_ampdu_param(mode, value);     
        return; 
    }
USAGE:
    LOG_PRINTF("ampdu tx 0/1, ampdu lastretryb 0/1\r\n");
    LOG_PRINTF("ampdu lastbrate [0-3], ampdu lastbonce 0/1\r\n");
    LOG_PRINTF("ampdu sessionchk 0/1, ampdu maxretry [2-6]\r\n");
    LOG_PRINTF("ampdu maintry [1-3], ampdu rx 0/1\r\n");
}
