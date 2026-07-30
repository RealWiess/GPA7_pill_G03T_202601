
/*********************
 *      INCLUDES
 *********************/
#include "project.h"
#include "drv_l2_sccb.h"
#include "drv_l1_gpio.h"
#include "drv_l1_timer.h"
#include "gplib_mm_gplus.h"
#include "gplib_print_string.h"
#include "lv_port_touch.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
#if defined(RUN_ON_HMI_BOARD) && (RUN_ON_HMI_BOARD == 1)
    #define TOUCH_IO_RESET          IO_C5   // same as LCD_RESET
#else
    #define TOUCH_IO_RESET          IO_B15   // same as LCD_RESET
#endif

//#define GT9147_DevAdr0 			0x28
#define GT9147_DevAdr1 			0xBA

/* GT9147 registers */
//#define GT9147_ConfigMsgReg 	0x8047			//start register of touch config, 0x8047~0x8100
#define GT9147_ProductIDReg		0x8140			//start register of product ID, 4 bytes
#define GT9147_TouchStateReg 	0X814E			//register of touch state
#define GT9147_TouchPoint1Reg	0X8150  		//start register of touch point 1, 4 bytes
#define GT9147_TouchPoint2Reg	0X8158  		//start register of touch point 2, 4 bytes
#define GT9147_TouchPoint3Reg	0X8160  		//start register of touch point 3, 4 bytes
#define GT9147_TouchPoint4Reg	0X8168  		//start register of touch point 4, 4 bytes
#define GT9147_TouchPoint5Reg	0X8170  		//start register of touch point 5, 4 bytes

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
//touch point registers
static void *touch_handle;
const INT16U GT9147_TPR_TBL[5] = {GT9147_TouchPoint1Reg,GT9147_TouchPoint2Reg,GT9147_TouchPoint3Reg,GT9147_TouchPoint4Reg,GT9147_TouchPoint5Reg};

/**********************
 *      MACROS
 **********************/
void touch_reset(INT8U pol,INT16U dlt)
{
    if(pol==0)//low reset
    {
        gpio_write_io(TOUCH_IO_RESET,1);
        gpio_write_io(TOUCH_IO_RESET,0);
        drv_msec_wait(dlt);
        gpio_write_io(TOUCH_IO_RESET,1);
    }else{
        gpio_write_io(TOUCH_IO_RESET,0);
        gpio_write_io(TOUCH_IO_RESET,1);
        drv_msec_wait(dlt);
        gpio_write_io(TOUCH_IO_RESET,0);
    }
}

INT32S touch_init(void)
{
	sccb_config_t cfg;
	INT32S ret;

	touch_reset(0,10);

	cfg.scl_port = IO_B4;   //i2c0 scl
	cfg.scl_drv = IOD_DRV_30mA;
	cfg.sda_port = IO_B5;   //i2c0 sda
	cfg.sda_drv = IOD_DRV_30mA;
	cfg.pwdn_port = 0;
	cfg.pwdn_drv = 0;
	cfg.have_pwdn = 0;
	cfg.RegBits = 16;
	cfg.DataBits = 8;
	cfg.slaveAddr = GT9147_DevAdr1;
	cfg.timeout = 0x20000;
	cfg.clock_rate = 100;

	touch_handle = drv_l2_sccb_open_ext(&cfg);

	if(touch_handle == 0) {
		DBG_PRINT("Sccb open fail.\r\n");
		return STATUS_FAIL;
	}

    //clear touch state register
    ret = drv_l2_sccb_write(touch_handle, GT9147_TouchStateReg, 0);
    if(ret < 0) {
        gp_free(touch_handle);
        touch_handle = NULL;
    }

	return ret;
}

INT32S touch_scan(TouchPointRef_t *TPR_Structure)
{
	INT8U i, tp_cnt=0, sta[1], dat[4]={0};
	INT32S ret=-1;

	if(touch_handle == NULL)
        return ret;

	ret = drv_l2_sccb_read(touch_handle, GT9147_TouchStateReg, (INT16U *)sta);     //read touch state
	if(ret < 0) {
        DBG_PRINT("Sccb read fail.\r\n");
        gp_free(touch_handle);
        touch_handle = NULL;
        return ret;
	}
    //bit7=1, valid touch point
	if(sta[0] & 0x80) {
        //DBG_PRINT("touch state: 0x%x\r\n", sta[0]);
        //bit3~0 = number of touch points
		if(sta[0] & 0x0F) {
            tp_cnt = sta[0] & 0x0F;
			TPR_Structure->TouchSta = ~(0xFF << tp_cnt);	//~(0xFF << (sta & 0x0F)) convert number of touch points to touch flag
			for(i=0;i<5;i++)
			{
                //check whether touch point 1~5 is pressed
				if (TPR_Structure->TouchSta & (1<<i)) {
					drv_l2_sccb_continue_read(touch_handle, GT9147_TPR_TBL[i], dat, 4); //read touch point (x, y)
					TPR_Structure->x[i] = ((uint16_t)(dat[1]) << 8) + dat[0];
                    TPR_Structure->y[i] = ((uint16_t)(dat[3]) << 8) + dat[2];
					//DBG_PRINT("touch (%d,%d)\r\n", TPR_Structure->x[i], TPR_Structure->y[i]);
				}
			}
		}
		/* need to write 0x0 to GT9147_TouchStateReg after reading.
		 otherwise, touch point can not be read next time, which means GT9147_TouchStateReg bit7~0 = 0. */
		sta[0] = 0;
		drv_l2_sccb_write(touch_handle, GT9147_TouchStateReg, sta[0]);
		//clear touch flag
		TPR_Structure->TouchSta &= 0xE0;	//bit4~0
	}

    return tp_cnt;
}
