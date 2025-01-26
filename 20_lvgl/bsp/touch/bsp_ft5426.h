//
// Created by 86157 on 2025/1/17.
//

#ifndef INC_18_TOUCH_FT5426_BSP_FT5426_H
#define INC_18_TOUCH_FT5426_BSP_FT5426_H

typedef struct {
    unsigned char initfalg;        /* 触摸屏初始化状态 */
    unsigned char intflag;        /* 标记中断有没有发生 */
    unsigned char point_num;    /* 触摸点 		*/
    unsigned short x[5];        /* X轴坐标 	*/
    unsigned short y[5];        /* Y轴坐标 	*/
} ft5426_device_t;

extern ft5426_device_t ft5426_device;

void Bsp_FT5426_Init(void);

void Bsp_FT5426_Test(void);

#endif //INC_18_TOUCH_FT5426_BSP_FT5426_H
