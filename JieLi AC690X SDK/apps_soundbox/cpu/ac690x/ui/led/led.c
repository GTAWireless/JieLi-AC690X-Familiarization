#include "ui/led/led.h"
#include "timer.h"
#include "rtc/rtc_api.h"

extern tbool is_sniff_mode();
u32 g_led_fre = 0; ///<led fre
u8 g_led_flash = 0;///<b_led and r_led flash
extern bool g_ad_led_flag;
extern u8 power_off_flag;
/*----------------------------------------------------------------------------*/
/**@brief   LED��ʼ������
   @param   void
   @return  void
   @note    void led_init(void)
*/
/*----------------------------------------------------------------------------*/
void led_init(void)
{
#if LED_EN
    LED_INIT_EN();
    //B_LED_ON();
    //R_LED_ON();
    s32 ret;
	ret = timer_reg_isr_fun(timer0_hl,5,led_scan,NULL);
	if(ret != TIMER_NO_ERR)
	{
		printf("led_scan err = %x\n",ret);
	}
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief   LED�رպ���
   @param   void
   @return  void
   @note    void led_close(void)
*/
/*----------------------------------------------------------------------------*/
void led_close(void)
{
    LED_INIT_DIS();
}

/*----------------------------------------------------------------------------*/
/**@brief   LED�򿪺���
   @param   void
   @return  void
   @note    void led_open(void)
*/
/*----------------------------------------------------------------------------*/
void led_open(void)
{
    LED_INIT_EN();
}

/*----------------------------------------------------------------------------*/
/**@brief   LEDƵ�����ú���(50ms*fre = ʵ����˸Ƶ��)
   @param   fre:led��˸Ƶ�� ��ע����fre = 0ʱ����ʾled������
            led_flash:��ʾB_led��R_led�Ƿ���
   @return  void
   @note    ʵ����˸Ƶ�� = fre*5*scan_fre
            ����scan_fre = 10ms
                led_fre_set(10,0)
            ��
                ledʵ����˸Ƶ�� = 10*5*10 = 500ms = 0.5s
*/
/*----------------------------------------------------------------------------*/
void led_fre_set(u32 fre,u8 led_flash)
{
    g_led_fre = fre*5;
    g_led_flash = led_flash;
}
volatile u8 led_rg_flag=0;
u32 led_on_sniff(u32 led_flash)
{
    if(!g_led_fre) /*�����Ƴ���*/
	{
        B_LED_OFF();
        R_LED_OFF();
			
    }
	else if(led_flash)/*�����ƻ���*/
	{
		if(led_rg_flag)
		 {
		     R_LED_ON();
		     B_LED_OFF();
		     led_rg_flag=0;
		 }
		else
		{	
		    B_LED_ON();
		    R_LED_OFF();
		    led_rg_flag=1;
		 }	

    }
	else
	{ /*���ƶ���*/
        B_LED_ON();
        R_LED_OFF();

	}
	return 1;	
}
u32 led_off_sniff(u32 led_flash)
{
    if(!g_led_fre) /*�����Ƴ���*/
	{
        B_LED_OFF();
        R_LED_OFF();
			
    }
	else if(led_flash)
	{ /*�����ƻ���*/
        B_LED_OFF();
        R_LED_OFF();

    }
	else
	{ /*���ƶ���*/
        B_LED_OFF();
        R_LED_OFF();
	}
	return 1;	
}


/*----------------------------------------------------------------------------*/
/**@brief   ledɨ�躯��
   @param   void
   @return  void
   @note    void led_scan(void *param)
*/
/*----------------------------------------------------------------------------*/
extern u8 is_pwrtimer_en();
extern volatile u8 low_power_flag;
extern void pwr_timer_control_led(u8 led_flash_mode,u32 led_on_ms,u32 led_off_ms,u32 (*led_off)(u32 mode) , u32 (*led_on)(u32 mode));
extern void pwr_timer_control_led_close();
void led_scan(void *param)
{
#if LED_EN
    static u32 led_cnt;

	if(g_ad_led_flag)return;

    if(is_sniff_mode()||is_pwrtimer_en())
	{
        if(!g_led_fre) /*�����Ƴ���*/
		{
			/*
			         led_on_sniff     led_off_sniff    led_on_sniff
				  |_______________|_________________|_____________
				  |     0(20)     |     0(20)       |   0(20)
		          |               |                 |

			*/
			pwr_timer_control_led(0,0,0,led_off_sniff ,led_on_sniff);
				
        }
		else if(g_led_flash)
		{ /*�����ƻ���*/
			/*
			       led_on_sniff(��)                    led_on_sniff(��)
			       _________________                  ___________________
				  |(g_led_fre*2*10) |led_off_sniff 50|  (g_led_fre*2*10  |led_off_sniff 50
			______|                 |________________|                   |__________________

			*/
			pwr_timer_control_led(g_led_flash,(g_led_fre*2*5),50,led_off_sniff ,led_on_sniff);///
        }
		else
		{ /*���ƶ���*/

			/*
			       led_on_sniff(��)                                  led_on_sniff(��)
			       _______________                                 _________________
				  |     (10*10)   |  led_off_sniff (g_led_fre*10) |   (10*10) 
			______|               |_______________________________|

			*/
			pwr_timer_control_led(g_led_flash,10*10,g_led_fre*10,led_off_sniff ,led_on_sniff);
		}
        return;
	}
	else
	{
        pwr_timer_control_led_close();
	}
    if(!g_led_fre)
	{ /*�����Ƴ���*/
        led_cnt = 0;
        B_LED_ON();
        R_LED_ON();
    }
	else if(g_led_flash)
	{ /*�����ƻ���*/
        led_cnt++;
        if(led_cnt > g_led_fre)
		{
            if(led_cnt >= g_led_fre*2)
                led_cnt = 0;
            	B_LED_ON();
           // R_LED_OFF();
        }
		else if(led_cnt > 0)
		{
            B_LED_OFF();
            R_LED_ON();
        }
    }
	else
	{ /*���ƶ���*/
        led_cnt++;
        R_LED_OFF();
        if(led_cnt > g_led_fre)
		{
            if(led_cnt >= g_led_fre*2)
                led_cnt = 0;
			if(power_off_flag)
			{
            	B_LED_OFF();
			}
			else
			{
            	B_LED_ON();
			}
        }
        else if(led_cnt > 0)
		{
            B_LED_OFF();
        }
    }
#endif
}




