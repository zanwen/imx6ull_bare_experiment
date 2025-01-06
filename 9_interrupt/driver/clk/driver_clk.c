#include "driver_clk.h"

void Driver_CLK_Init() {
    /*
        CCM CCSR: pll1_sw_clk_sel bit2
            pll1_main_clk 0
            step_clk 1

        CCM CCSR: step_sel bit8
            osc_clk 0

        CCM_ANALOG CCM_ANALOG_PLL_ARMn
            ENABLE bit13
            DIV_SELECT bit0-6
                PLL output frequency = Fref * DIV_SEL/2
                PLL = 792M
                Fref 参考时钟为外部晶振24M
                => DIV_SEL = 66

        CCM CACRR[ARM_PODF] bit0-2
            PLL1 => /(ARM_PODF+1) => ARM_CLK_ROOT
     */
    /* 初始化 ARM 内核时钟源 ARM_CLK_ROOT */
    uint32_t tmp;
    if ((CCM->CCSR & (1 << 2)) == 0) { // 如果当前pll1选择的是 pll1_main_clk，则先临时切换到step_clk并使用osc
        CCM->CCSR &= ~(1 << 8); // setp_clk 选择 24M osc
        CCM->CCSR |= (1 << 2);  // pll1 临时切为 step_clk
    }
    CCM_ANALOG->PLL_ARM |= (1 << 13) | ((66 << 0) & 0x7F); // 使能PLL_ARMn并配置DIV_SELECT，PLL1 = 792M
    CCM->CCSR &= ~(1 << 2);                                // pll1 切换回 pll1_main_clk

    CCM->CACRR = 0; // PLL1 /(0+1) => ARM_CLK_ROOT => 792M

    tmp = CCM_ANALOG->PFD_528;
    tmp &= ~0x3F3F3F3F;
    tmp |= (27 << 0) | (16 << 8) | (24 << 16) | (32 << 24);
    CCM_ANALOG->PFD_528 = tmp;

    tmp = CCM_ANALOG->PFD_480;
    tmp &= ~0x3F3F3F3F;
    tmp |= (12 << 0) | (16 << 8) | (17 << 16) | (19 << 24);
    CCM_ANALOG->PFD_480 = tmp;

    // CCM_ANALOG->PFD_528 &= ~0x3F3F3F3F;
    // CCM_ANALOG->PFD_528 |= (27 << 0) | (16 << 8) | (24 << 16) | (32 << 24);

    // CCM_ANALOG->PFD_480 &= ~0x3F3F3F3F;
    // CCM_ANALOG->PFD_480 |= (12 << 0) | (16 << 8) | (17 << 16) | (19 << 24);

    /* 配置 PERCLK 和 IPG ， 以及依赖的AHB */
    // AHB最大频率132M，可以由PLL2 PFD2 396M/3 得到
    CCM->CBCMR &= ~(0x3 << 18); // CBCMR[PRE_PERIPH_CLK_SEL]
    CCM->CBCMR |= (0x1 << 18);
    CCM->CBCDR &= ~(0x1 << 25); // CBCDR[PERIPH_CLK_SEL]
    while (CCM->CDHIPR & (1 << 5))
        ;

    // 配置AHB分频时需要关闭AHB时钟原输出；BOOT ROM已经设置了AHB分频为3
    // CCM->CBCDR &= ~(0x7 << 10); // AHB_PODF
    // CCM->CBCDR |= (0x2 << 10);  // 3分频 396/3=132
    // while (BIT_IS_SET(CCM->CDHIPR, 1))
    //     ;

    /* 配置IPG */
    CCM->CBCDR &= ~(0x3 << 8); // CBCDR[IPG_PODF]
    CCM->CBCDR |= (0x1 << 8);  // 二分频 132/2=66

    /* 配置PERCLK */
    CCM->CSCMR1 &= ~(1 << 6);    // 从 IPG时钟源派生时钟
    CCM->CSCMR1 &= ~(0x3F << 6); // 不分频 66/1=66
}

void Driver_CLK_Enable() {
    CCM->CCGR0 = 0xFFFFFFFF;
    CCM->CCGR1 = 0xFFFFFFFF;
    CCM->CCGR2 = 0xFFFFFFFF;
    CCM->CCGR3 = 0xFFFFFFFF;
    CCM->CCGR4 = 0xFFFFFFFF;
    CCM->CCGR5 = 0xFFFFFFFF;
    CCM->CCGR6 = 0xFFFFFFFF;
}
