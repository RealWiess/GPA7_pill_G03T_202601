/**
 * @file lv_port_fs_templ.c
 *
 */

/*Copy this file as "lv_port_fs.c" and set this value to "1" to enable content*/

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_fs.h"
#include "lvgl.h"
#include "fs.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if 0
typedef struct _lv_fs_drv_t {
    char letter;
    uint16_t cache_size;
    bool (*ready_cb)(struct _lv_fs_drv_t * drv);

    void * (*open_cb)(struct _lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
    lv_fs_res_t (*close_cb)(struct _lv_fs_drv_t * drv, void * file_p);
    lv_fs_res_t (*read_cb)(struct _lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
    lv_fs_res_t (*write_cb)(struct _lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw);
    lv_fs_res_t (*seek_cb)(struct _lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
    lv_fs_res_t (*tell_cb)(struct _lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);

    void * (*dir_open_cb)(struct _lv_fs_drv_t * drv, const char * path);
    lv_fs_res_t (*dir_read_cb)(struct _lv_fs_drv_t * drv, void * rddir_p, char * fn);
    lv_fs_res_t (*dir_close_cb)(struct _lv_fs_drv_t * drv, void * rddir_p);

#if LV_USE_USER_DATA
    void * user_data; /**< Custom file user data*/
#endif
} lv_fs_drv_t;
#endif
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
char cpath[]=":\\";
char *root_path;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
char *get_fs_root_path(INT8U drv)
{
    if (drv == FS_SD)
        return "C:\\";
    else if (drv == FS_SD2)
        return "K:\\";
    else if (drv == FS_NAND1)
        return "A:\\";
    else if (drv == FS_NAND2)
        return "B:\\";
    else if (drv == FS_NAND3)
        return "M:\\";
    else if (drv == FS_NAND4)
        return "E:\\";
    else if (drv == FS_NOR)
        return "H:\\";
    else if (drv == FS_SD1)
        return "F:\\";
    else
    {
        DBG_PRINT("Unknown drive %u.\r\n", drv);
        return NULL;
    }
}


//    void * (*open_cb)(struct _lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode);
static void * gp_fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    int32_t ret=0;
    char f_path[64] = "";
    char c_root[2]="\0";

    // 調用自定義檔案系統驅動程式的打開函數 gp_fs_open()，並返回結果
    c_root[0] = drv->letter;
    strcpy(f_path, c_root);
    strcat(f_path, cpath);
    strcat(f_path, path);

    ret = fs_open(f_path, mode);

    if(ret >= 0)
    {
        LV_LOG_USER("fo:%s:%d\r\n",f_path,ret);
        return (void *)ret;
    }

    DBG_PRINT("fopen fail:%d path:%s\r\n",ret,f_path);
    return (void *)(ret);
}

//    lv_fs_res_t (*read_cb)(struct _lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br);
#if 0
btr：代表 "bytes to read"，即要讀取的字節數。它表示要從檔案中讀取的資料的大小。
br：代表 "bytes read"，即已讀取的字節數。它是一個指向 uint32_t 的指標，用於接收實際讀取的字節數。在 read_cb 函數中，你需要將實際讀取的字節數賦值給 br，以便通知 LVGL 實際讀取了多少字節的資料。
簡單來說，btr 是指定要從檔案中讀取的字節數，而 br 是實際讀取的字節數，你需要在 read_cb 函數中將實際讀取的字節數賦值給 br。
#endif
static lv_fs_res_t gp_fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br)
{
    int8_t res_t = 0;
    int32_t res_fs = 0;
    // 調用自定義檔案系統驅動程式的讀取函數 gp_fs_read()，並返回結果
    res_fs = fs_read((long)file_p, (INT32U)buf, btr);
    if(res_fs != (-1))
    {
        *br = res_fs;
        //DBG_PRINT("f_r:[%d:%d] ",file_p,*br);
        //DBG_PRINT("fr[%d] rbf:%x rb:%d ",file_p,buf,*br);
        //DBG_PRINT("fr[%d]rl[%x] ",file_p,*br);
    }else{
        res_t = LV_FS_RES_FS_ERR;
    }
    return res_t;
}

static lv_fs_res_t gp_fs_write(lv_fs_drv_t * drv, void * file_p, const void * buf, uint32_t btw, uint32_t * bw)
{
    // 調用自定義檔案系統驅動程式的寫入函數 gp_fs_write()，並返回結果
    bw = (uint32_t *)fs_write((long)file_p, (INT32U)buf, btw);
    return 0;
}
//lv_fs_res_t (*tell_cb)(struct _lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p);
//lv_fs_res_t lv_fs_tell(lv_fs_file_t * file_p, uint32_t *
//res = file_p->drv->tell_cb(file_p->drv, file_p->file_d, pos);
//static int32_t gp_fs_tell(lv_fs_file_t * file_p, uint32_t * pos)
static lv_fs_res_t gp_fs_tell(struct _lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
#if 0
    struct sfn_info file_info;

    sfn_stat((long)file_p, &file_info);
    //DBG_PRINT("file size %lu bytes.\r\n",(INT32U)file_info.f_size);
    *pos_p = file_info.f_size;
#endif
    *pos_p = fs_lseek64((long)file_p, 0, SEEK_CUR);
    return (LV_FS_RES_OK);

}
//lv_fs_res_t (*seek_cb)(struct _lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t gp_fs_seek(struct _lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    int32_t ret=0;

    ret = fs_lseek((long)file_p, pos,whence);
    if(ret != (-1))
        return (0);
    else
        return (-1);
}

static lv_fs_res_t gp_fs_close(lv_fs_drv_t * drv, void * file_p)
{
    //DBG_PRINT("fc[%d]\r\n\n",file_p);
    return fs_close((long)file_p);
}

static INT16S DiskMount()
{
    int i = 3;
	while(i>0)
	{
        i--;
        DBG_PRINT("Try diskMount ... [%d]\r\n", HMI_DEMO_CARD_SEL);
		if( _devicemount(HMI_DEMO_CARD_SEL))
		{
			DBG_PRINT("Mount Disk Fail[%d]\r\n", HMI_DEMO_CARD_SEL);
			_deviceunmount(HMI_DEMO_CARD_SEL);
		}
		else
		{
			DBG_PRINT("Mount Disk success[%d]\r\n", HMI_DEMO_CARD_SEL);
            // set fs root path
            chdir(FS_ROOT_PATH);

            return 0;
			//break;
		}
	}

    return -1;
}

INT16S lv_fs_gp_init(void)
{
    INT16S ret=0;

    ret = DiskMount();
    if(ret==0)
        DBG_PRINT("mount(%d) OK.\r\n",HMI_DEMO_CARD_SEL);
    else{
        DBG_PRINT("mount(%d) FAIL!!(%d)\r\n",HMI_DEMO_CARD_SEL,ret);
        return(ret);
    }
    // 定義和初始化自定義檔案系統驅動程式
    static lv_fs_drv_t gp_fs_drv;
    lv_fs_drv_init(&gp_fs_drv);

    // 設定自定義檔案系統驅動程式的名稱
    gp_fs_drv.letter = FS_LETTER;
    gp_fs_drv.cache_size = GP_FS_FATFS_CACHE_SIZE;
    // 設定自定義檔案系統驅動程式的回調函數
    gp_fs_drv.open_cb = gp_fs_open;
    gp_fs_drv.read_cb = gp_fs_read;
    gp_fs_drv.seek_cb = gp_fs_seek;
    gp_fs_drv.tell_cb = gp_fs_tell;
    gp_fs_drv.write_cb = gp_fs_write;
    gp_fs_drv.close_cb = gp_fs_close;

    // 註冊自定義檔案系統驅動程式
    lv_fs_drv_register(&gp_fs_drv);
    return ret;
}

