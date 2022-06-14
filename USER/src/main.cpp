#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/examples/lv_examples.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/display/sunxifb.h"
#include "lv_drivers/indev/evdev.h"
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "App.h"
#include "Common/HAL/HAL.h"
#include "player_int.h"
#define DISP_BUF_SIZE (1920 * 1080)

extern "C"
{
    LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
}
const char *mp4add = "https://sf1-hscdn-tos.pstatp.com/obj/media-fe/xgplayer_doc_video/mp4/xgplayer-demo-720p.mp4";
const char *rtspaddr = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov";
const char *cctv1addr = "http://39.134.115.163:8080/PLTV/88888910/224/3221225618/index.m3u8";

static void
HALfbinit(void)
{
#if USE_SUNXIFB
    uint32_t rotated = LV_DISP_ROT_NONE;
    /*Linux frame buffer device init*/

    sunxifb_init(rotated);
    /*A buffer for LittlevGL to draw the screen's content*/
    static uint32_t width, height;
    sunxifb_get_sizes(&width, &height);

    static lv_color_t *buf1, *buf2;
    buf1 = (lv_color_t *)malloc(DISP_BUF_SIZE * sizeof(lv_color_t));
    buf2 = (lv_color_t *)malloc(DISP_BUF_SIZE * sizeof(lv_color_t));
    if (buf1 == NULL || buf2 == NULL)
    {
        sunxifb_exit();
        printf("malloc draw buffer fail\n");
        return 0;
    }
    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = sunxifb_flush;
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;
    disp_drv.rotated = rotated;
    lv_disp_drv_register(&disp_drv);
#else
    fbdev_init();
    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf1[DISP_BUF_SIZE];
    static lv_color_t buf2[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    disp_drv.hor_res = 1920;
    disp_drv.ver_res = 1080;
    disp_drv.screen_transp = 1;
    lv_disp_drv_register(&disp_drv);
    lv_obj_set_style_bg_opa(lv_scr_act(), 0, 0);
    lv_disp_set_bg_opa(NULL, LV_OPA_TRANSP);
#endif
    evdev_init();
    int ret = evdev_set_file("/dev/input/event1");
    LV_LOG_USER("ret=%d", ret);

    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;
    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = evdev_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    lv_group_t *group = lv_group_create();
    lv_indev_set_group(mouse_indev, group);
    lv_group_set_default(group);

    /*Set a cursor for the mouse*/
    lv_obj_t *cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);     /*Set the image source*/
    lv_indev_set_cursor(mouse_indev, cursor_obj);       /*Connect the image  object to the driver*/
}

void tplayertest1(void)
{

    tplayer_init(TPLAYER_VIDEO_ROTATE_DEGREE_0);
    tplayer_play_url(cctv1addr);
    tplayer_setlooping(1);
    tplayer_play();
}

int main(int argc, const char *argv[])
{
    lv_init();
    lv_fs_posix_init();
    HALfbinit();
    /*Create a Demo*/
    // lv_demo_widgets();
    //   lv_demo_stress();
    // HAL::HAL_Init();
    // App_Init();
    //  lv_demo_benchmark();
    tplayertest1();
    /*Handle LitlevGL tasks (tickless mode)*/
    LV_LOG_USER("disbuff:%d,%d", DISP_BUF_SIZE, USE_SUNXIFB);
    while (1)
    {
        lv_timer_handler();
        HAL::HAL_Update();
        usleep(5000);
    }
    App_Uninit();
    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
extern "C" uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if (start_ms == 0)
    {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
