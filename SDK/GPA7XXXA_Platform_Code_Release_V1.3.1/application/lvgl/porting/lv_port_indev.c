/**
 * @file lv_port_indev_templ.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "lvgl.h"
#include "drv_l2_ad_key_scan.h"
#include "lv_port_touch.h"


/*********************
 *      DEFINES
 *********************/
#define KEY_UP      IO_B8 //IO_D7
#define KEY_OK      IO_B9 //IO_D8
#define KEY_DOWN    IO_B10 //IO_D9

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if INPUT_TOUCH_PAD
static INT32S touchpad_init(void);
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
#endif

#if INPUT_IO_KEY
static void keypad_init(void);
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t keypad_get_key(void);
#endif

#if INPUT_ADC_KEY
static void button_init(void);
static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static int8_t button_get_pressed_id(void);
//static bool button_is_pressed(uint8_t id);
#endif

#if 0
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y);

static void mouse_init(void);
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static bool mouse_is_pressed(void);
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);

static void encoder_init(void);
static void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static void encoder_handler(void);
#endif

/**********************
 *     VARIABLES
 **********************/
lv_indev_t * indev_touchpad;
lv_indev_t * indev_mouse;
lv_indev_t * indev_keypad;
lv_indev_t * indev_encoder;
lv_indev_t * indev_button;

lv_group_t * group_keypad;
//static int32_t encoder_diff;
//static lv_indev_state_t encoder_state;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
TouchPointRef_t tp = {0,{0},{0}};

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LittelvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

#if INPUT_TOUCH_PAD
    /*------------------
     * Touchpad
     * -----------------*/
    static lv_indev_drv_t indev_drv_touch;

    /*Initialize your touchpad if you have*/
    INT32S ret;
    ret = touchpad_init();
    if(ret >= 0)
    {
        /*Register a touchpad input device*/
        lv_indev_drv_init(&indev_drv_touch);
        indev_drv_touch.type = LV_INDEV_TYPE_POINTER;
        indev_drv_touch.read_cb = touchpad_read;
        indev_touchpad = lv_indev_drv_register(&indev_drv_touch);
    }
#endif

#if INPUT_IO_KEY
    /*------------------
     * Keypad
     * -----------------*/
    static lv_indev_drv_t indev_drv_keypad;

    /*Initialize your keypad or keyboard if you have*/
    keypad_init();

    /*Register a keypad input device*/
    lv_indev_drv_init(&indev_drv_keypad);
    indev_drv_keypad.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_keypad.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv_keypad);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/
     group_keypad = lv_group_create();
     lv_indev_set_group(indev_keypad, group_keypad);
#endif

#if INPUT_ADC_KEY
    /*------------------
     * Button
     * -----------------*/
    static lv_indev_drv_t indev_drv_button;

    /*Initialize your button if you have*/
    button_init();

    /*Register a button input device*/
    lv_indev_drv_init(&indev_drv_button);
    indev_drv_button.type = LV_INDEV_TYPE_BUTTON;
    indev_drv_button.read_cb = button_read;
    indev_button = lv_indev_drv_register(&indev_drv_button);

    /*Assign buttons to points on the screen*/
    static const lv_point_t btn_points[5] = {
        {445, 535}, /* Button 0 -> x:445; y:535 -> left direction */
        {835, 535}, /* Button 1 -> x:835; y:535 -> right direction */
        {910, 420}, /* Button 2 -> x:910; y:420 -> light */
        {905, 300},  /* Button 3 -> x:905; y:300 -> warning/speed */
        {60,25}     /* left top button */
    };
    lv_indev_set_button_points(indev_button, btn_points);
#endif

#if 0
    /*------------------
     * Mouse
     * -----------------*/
    static lv_indev_drv_t indev_drv_mouse;

    /*Initialize your mouse if you have*/
    mouse_init();

    /*Register a mouse input device*/
    lv_indev_drv_init(&indev_drv_mouse);
    indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
    indev_drv_mouse.read_cb = mouse_read;
    indev_mouse = lv_indev_drv_register(&indev_drv_mouse);

    /*Set cursor. For simplicity set a HOME symbol now.*/
    lv_obj_t * mouse_cursor = lv_img_create(lv_scr_act());
    lv_img_set_src(mouse_cursor, LV_SYMBOL_HOME);
    lv_indev_set_cursor(indev_mouse, mouse_cursor);

    /*------------------
     * Encoder
     * -----------------*/
    static lv_indev_drv_t indev_drv_encoder;

    /*Initialize your encoder if you have*/
    encoder_init();

    /*Register a encoder input device*/
    lv_indev_drv_init(&indev_drv_encoder);
    indev_drv_encoder.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_encoder.read_cb = encoder_read;
    indev_encoder = lv_indev_drv_register(&indev_drv_encoder);

    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_encoder, group);`*/
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if INPUT_TOUCH_PAD
/*------------------
 * Touchpad
 * -----------------*/

/*Initialize your touchpad*/
static INT32S touchpad_init(void)
{
    INT32S ret;

    /*Your code comes here*/
    ret = touch_init();
    if(ret < 0)
    {
        DBG_PRINT("Touch Init=>%d\r\n",ret);
    }

    return ret;
}

/*Will be called by the library to read the touchpad*/
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    INT32S ret=0;

    /*Save the pressed coordinates and the state*/
    //if(touchpad_is_pressed()) {
    ret = touch_scan(&tp);
    if(ret < 0) {
        lv_indev_delete(indev_touchpad);
    }

    if(ret){
        //touchpad_get_xy(&last_x, &last_y);

        last_x = tp.x[0];
        last_y = tp.y[0];
        data->state = LV_INDEV_STATE_PR;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Set the last pressed coordinates*/
    data->point.x = last_x;
    data->point.y = last_y;
}
#endif

#if INPUT_IO_KEY
/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/
static void keypad_init(void)
{
    /*Your code comes here*/
    gpio_init_io(KEY_UP, GPIO_INPUT);
    gpio_init_io(KEY_OK, GPIO_INPUT);
    gpio_init_io(KEY_DOWN, GPIO_INPUT);
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    //mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;
        //DBG_PRINT("key:%d\r\n", act_key);

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
            case 1:
                act_key = LV_KEY_PREV;
                break;
            case 2:
                act_key = LV_KEY_ENTER;
                break;
            case 3:
                act_key = LV_KEY_NEXT;
                break;
            case 4:
                act_key = LV_KEY_LEFT;
                break;
            case 5:
                act_key = LV_KEY_RIGHT;
                break;
        }

        last_key = act_key;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/
    if(gpio_read_io(KEY_UP))
        return 1;
    else if(gpio_read_io(KEY_OK))
        return 2;
    else if(gpio_read_io(KEY_DOWN))
        return 3;

    return 0;
}
#endif

#if INPUT_ADC_KEY
/*------------------
 * Button
 * -----------------*/

/*Initialize your buttons*/
static void button_init(void)
{
    /*Your code comes here*/
    adc_key_scan_init();
}

/*Will be called by the library to read the button*/
static void button_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    static uint8_t last_btn = 0;

    /*Get the pressed button's ID*/
    int8_t btn_act = button_get_pressed_id();

    if(btn_act >= 0) {
        data->state = LV_INDEV_STATE_PR;
        last_btn = btn_act;
        //DBG_PRINT("btn:%d\r\n", btn_act);
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }

    /*Save the last pressed button's ID*/
    data->btn_id = last_btn;
}

/*Get ID  (0, 1, 2 ..) of the pressed button*/
static int8_t button_get_pressed_id(void)
{
    //uint8_t i;

    if(adc_key_scan_timeout(0) == 0)
    {
        /*Check to buttons see which is being pressed (assume there are 2 buttons)*/
        //for(i = 0; i < 2; i++) {
        //    /*Return the pressed button's ID*/
        //    if(button_is_pressed(i)) {
        //        return i;
        //    }
        //}
        if(ADKEY_IO1)
            return 0;
        else if(ADKEY_IO2)
            return 1;
        else if(ADKEY_IO3)
            return 2;
        else if(ADKEY_IO4)
            return 3;
        else if(ADKEY_IO5)
            return 4;
        else if(ADKEY_IO6)
            return 5;
        else if(ADKEY_IO7)
            return 6;
        else if(ADKEY_IO8)
            return 7;
    }

    /*No button pressed*/
    return -1;
}

/*Test if `id` button is pressed or not*/
//static bool button_is_pressed(uint8_t id)
//{
//    /*Your code comes here*/
//    return false;
//}
#endif

#if 0
/*Return true is the touchpad is pressed*/
static bool touchpad_is_pressed(void)
{
    INT32S ret=0;
    /*Your code comes here*/
    ret = touch_scan(&tp);
    if(ret)
        return true;
    else
    return false;
}

/*Get the x and y coordinates if the touchpad is pressed*/
static void touchpad_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    INT32S ret=0;
    /*Your code comes here*/
    ret = touch_scan(&tp);
    if(ret > 0)
    {
        (*x) = tp.x[0];
        (*y) = tp.y[0];
    }
}

/*------------------
 * Mouse
 * -----------------*/

/*Initialize your mouse*/
static void mouse_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the mouse*/
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    /*Get the current x and y coordinates*/
    mouse_get_xy(&data->point.x, &data->point.y);

    /*Get whether the mouse button is pressed or released*/
    if(mouse_is_pressed()) {
        data->state = LV_INDEV_STATE_PR;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/*Return true is the mouse button is pressed*/
static bool mouse_is_pressed(void)
{
    /*Your code comes here*/

    return false;
}

/*Get the x and y coordinates if the mouse is pressed*/
static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y)
{
    /*Your code comes here*/

    (*x) = 0;
    (*y) = 0;
}

/*------------------
 * Encoder
 * -----------------*/

/*Initialize your keypad*/
static void encoder_init(void)
{
    /*Your code comes here*/
}

/*Will be called by the library to read the encoder*/
static void encoder_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{

    data->enc_diff = encoder_diff;
    data->state = encoder_state;
}

/*Call this function in an interrupt to process encoder events (turn, press)*/
static void encoder_handler(void)
{
    /*Your code comes here*/

    encoder_diff += 0;
    encoder_state = LV_INDEV_STATE_REL;
}
#endif

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
