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
    if(BIT_IS_NOSET(CCM->CCSR, 2)) { // 如果当前pll1选择的是pll1_main_clk，则先临时切换到step_clk并使用osc
        
        BIT_SET(CCM->CCSR, 2); // 切换为step_clk
    }
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

