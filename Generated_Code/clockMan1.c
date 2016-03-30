/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : clockMan1.c
**     Project     : DDScheduler
**     Processor   : MK64FN1M0VLL12
**     Component   : fsl_clock_manager
**     Version     : Component 1.3.0, Driver 01.00, CPU db: 3.00.000
**     Repository  : KSDK 1.3.0
**     Compiler    : GNU C Compiler
**     Date/Time   : 2016-03-30, 11:59, # CodeGen: 19
**
**     Copyright : 1997 - 2015 Freescale Semiconductor, Inc. 
**     All Rights Reserved.
**     
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**     
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**     
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**     
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**     
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**     
**     http: www.freescale.com
**     mail: support@freescale.com
** ###################################################################*/
/*!
** @file clockMan1.c
** @version 01.00
*/         
/*!
**  @addtogroup clockMan1_module clockMan1 module documentation
**  @{
*/         
/* clockMan1. */      
#include "clockMan1.h"
        
/*! @brief OSC Initialization Configuration Structure */  
osc_user_config_t clockMan1_osc0_Config = {
    .freq = 50000000U,
    .range = kOscRangeVeryHigh1,    
    .erefs = kOscSrcExt,
                
    /*! @brief Configuration for OSCERCLK */
    .oscerConfig = 
    {
        .enable = true,
        .enableInStop = false,
    },
}; 
                
/* ************************************************************************* 
 * Configuration structure for Clock Configuration 0
 * ************************************************************************* */
/*! @brief User Configuration structure SpeedMode0 */  
const clock_manager_user_config_t clockMan1_InitConfig0 = {
    /*! @brief Configuration of MCG */
    .mcgConfig =
    {
        .mcg_mode = kMcgModeFEI, /*!< FEI mode */
        /* ------------------ MCGIRCCLK settings ---------------------- */
        .irclkEnable = true, /*!< MCGIRCLK enable */
        .irclkEnableInStop = false, /*!< MCGIRCLK enable in stop mode */
        .ircs = kMcgIrcSlow, /*!< Slow internal reference clock selected */
        .fcrdiv = 0U,
                                
        /* -------------------- MCG FLL settings ---------------------- */
        .frdiv = 0, /*!< MCG_C1[FRDIV] */
        .drs = kMcgDcoRangeSelLow, /*!< MCG_C4[DRST_DRS] */
        .dmx32 = kMcgDmx32Default, /*!< MCG_C4[DMX32] */
        .oscsel = kMcgOscselOsc, /*!< Selects System Oscillator (OSCCLK) */
                                
        /* -------------------- MCG PLL settings ---------------------- */
        .pll0EnableInFllMode = false, /*!< PLL0 enable in FLL mode */
        .pll0EnableInStop = false, /*!< PLL0 enable in stop mode */
        .prdiv0 = 0U, /*!< PRDIV0 */
        .vdiv0 = 0U, /*!< VDIV0 */
    },       
                        
    /*! @brief Configuration of OSCERCLK */
    .oscerConfig = 
    {
        .enable = true, /*!< OSCERCLK enable or not */
        .enableInStop = false, /*!< OSCERCLK enable or not in stop mode */
    },
                          
    /*! @brief Configuration of SIM module */  
    .simConfig = 
    {
        .pllFllSel = kClockPllFllSelFll, /*!< Fll clock  */
        .er32kSrc = kClockEr32kSrcRtc, /*!< ERCLK32K source selection */
        .outdiv1 = 0U, /*!< OUTDIV1 setting */
        .outdiv2 = 0U, /*!< OUTDIV2 setting */
        .outdiv3 = 1U, /*!< OUTDIV3 setting */
        .outdiv4 = 1U, /*!< OUTDIV4 setting */
    }
};    
/* ************************************************************************* 
 * Configuration structure for Clock Configuration 1
 * ************************************************************************* */
/*! @brief User Configuration structure SpeedMode1 */  
const clock_manager_user_config_t clockMan1_InitConfig1 = {
    /*! @brief Configuration of MCG */
    .mcgConfig =
    {
        .mcg_mode = kMcgModePEE, /*!< PEE mode */
        /* ------------------ MCGIRCCLK settings ---------------------- */
        .irclkEnable = true, /*!< MCGIRCLK enable */
        .irclkEnableInStop = false, /*!< MCGIRCLK enable in stop mode */
        .ircs = kMcgIrcSlow, /*!< Slow internal reference clock selected */
        .fcrdiv = 0U,
                                
        /* -------------------- MCG FLL settings ---------------------- */
        .frdiv = 0, /*!< MCG_C1[FRDIV] */
        .drs = kMcgDcoRangeSelLow, /*!< MCG_C4[DRST_DRS] */
        .dmx32 = kMcgDmx32Default, /*!< MCG_C4[DMX32] */
        .oscsel = kMcgOscselOsc, /*!< Selects System Oscillator (OSCCLK) */
                                
        /* -------------------- MCG PLL settings ---------------------- */
        .pll0EnableInFllMode = true, /*!< PLL0 enable in FLL mode */
        .pll0EnableInStop = false, /*!< PLL0 enable in stop mode */
        .prdiv0 = 19U, /*!< PRDIV0 */
        .vdiv0 = 24U, /*!< VDIV0 */
    },       
                        
    /*! @brief Configuration of OSCERCLK */
    .oscerConfig = 
    {
        .enable = true, /*!< OSCERCLK enable or not */
        .enableInStop = false, /*!< OSCERCLK enable or not in stop mode */
    },
                          
    /*! @brief Configuration of SIM module */  
    .simConfig = 
    {
        .pllFllSel = kClockPllFllSelPll, /*!< Pll0 clock  */
        .er32kSrc = kClockEr32kSrcRtc, /*!< ERCLK32K source selection */
        .outdiv1 = 0U, /*!< OUTDIV1 setting */
        .outdiv2 = 1U, /*!< OUTDIV2 setting */
        .outdiv3 = 2U, /*!< OUTDIV3 setting */
        .outdiv4 = 4U, /*!< OUTDIV4 setting */
    }
};    
/* ************************************************************************* 
 * Configuration structure for Clock Configuration 2
 * ************************************************************************* */
/*! @brief User Configuration structure SpeedMode2 */  
const clock_manager_user_config_t clockMan1_InitConfig2 = {
    /*! @brief Configuration of MCG */
    .mcgConfig =
    {
        .mcg_mode = kMcgModeBLPI, /*!< BLPI mode */
        /* ------------------ MCGIRCCLK settings ---------------------- */
        .irclkEnable = true, /*!< MCGIRCLK enable */
        .irclkEnableInStop = false, /*!< MCGIRCLK enable in stop mode */
        .ircs = kMcgIrcFast, /*!< Fast internal reference clock selected */
        .fcrdiv = 0U, /*!< MCG_SC[FCRDIV] */
                                
        /* -------------------- MCG FLL settings ---------------------- */
        .frdiv = 0, /*!< MCG_C1[FRDIV] */
        .drs = kMcgDcoRangeSelLow, /*!< MCG_C4[DRST_DRS] */
        .dmx32 = kMcgDmx32Default, /*!< MCG_C4[DMX32] */
        .oscsel = kMcgOscselOsc, /*!< Selects System Oscillator (OSCCLK) */
                                
        /* -------------------- MCG PLL settings ---------------------- */
        .pll0EnableInFllMode = false, /*!< PLL0 enable in FLL mode */
        .pll0EnableInStop = false, /*!< PLL0 enable in stop mode */
        .prdiv0 = 0U, /*!< PRDIV0 */
        .vdiv0 = 0U, /*!< VDIV0 */
    },       
                        
    /*! @brief Configuration of OSCERCLK */
    .oscerConfig = 
    {
        .enable = true, /*!< OSCERCLK enable or not */
        .enableInStop = false, /*!< OSCERCLK enable or not in stop mode */
    },
                          
    /*! @brief Configuration of SIM module */  
    .simConfig = 
    {
        .pllFllSel = kClockPllFllSelIrc48M, /*!< IRC48MCLK  */
        .er32kSrc = kClockEr32kSrcRtc, /*!< ERCLK32K source selection */
        .outdiv1 = 0U, /*!< OUTDIV1 setting */
        .outdiv2 = 0U, /*!< OUTDIV2 setting */
        .outdiv3 = 0U, /*!< OUTDIV3 setting */
        .outdiv4 = 4U, /*!< OUTDIV4 setting */
    }
};    
/* ************************************************************************* 
 * Configuration structure for Clock Configuration 3
 * ************************************************************************* */
/*! @brief User Configuration structure SpeedMode3 */  
const clock_manager_user_config_t clockMan1_InitConfig3 = {
    /*! @brief Configuration of MCG */
    .mcgConfig =
    {
        .mcg_mode = kMcgModeBLPE, /*!< BLPE mode */
        /* ------------------ MCGIRCCLK settings ---------------------- */
        .irclkEnable = true, /*!< MCGIRCLK enable */
        .irclkEnableInStop = false, /*!< MCGIRCLK enable in stop mode */
        .ircs = kMcgIrcFast, /*!< Fast internal reference clock selected */
        .fcrdiv = 0U, /*!< MCG_SC[FCRDIV] */
                                
        /* -------------------- MCG FLL settings ---------------------- */
        .frdiv = 0, /*!< MCG_C1[FRDIV] */
        .drs = kMcgDcoRangeSelLow, /*!< MCG_C4[DRST_DRS] */
        .dmx32 = kMcgDmx32Default, /*!< MCG_C4[DMX32] */
        .oscsel = kMcgOscselRtc, /*!< Selects 32 kHz RTC Oscillator */
                                
        /* -------------------- MCG PLL settings ---------------------- */
        .pll0EnableInFllMode = false, /*!< PLL0 enable in FLL mode */
        .pll0EnableInStop = false, /*!< PLL0 enable in stop mode */
        .prdiv0 = 0U, /*!< PRDIV0 */
        .vdiv0 = 0U, /*!< VDIV0 */
    },       
                        
    /*! @brief Configuration of OSCERCLK */
    .oscerConfig = 
    {
        .enable = true, /*!< OSCERCLK enable or not */
        .enableInStop = false, /*!< OSCERCLK enable or not in stop mode */
    },
                          
    /*! @brief Configuration of SIM module */  
    .simConfig = 
    {
        .pllFllSel = kClockPllFllSelIrc48M, /*!< IRC48MCLK  */
        .er32kSrc = kClockEr32kSrcRtc, /*!< ERCLK32K source selection */
        .outdiv1 = 0U, /*!< OUTDIV1 setting */
        .outdiv2 = 0U, /*!< OUTDIV2 setting */
        .outdiv3 = 0U, /*!< OUTDIV3 setting */
        .outdiv4 = 0U, /*!< OUTDIV4 setting */
    }
};    
/* ************************************************************************* 
 * Configuration structure for Clock Configuration 4
 * ************************************************************************* */
/*! @brief User Configuration structure SpeedMode4 */  
const clock_manager_user_config_t clockMan1_InitConfig4 = {
    /*! @brief Configuration of MCG */
    .mcgConfig =
    {
        .mcg_mode = kMcgModePEE, /*!< PEE mode */
        /* ------------------ MCGIRCCLK settings ---------------------- */
        .irclkEnable = true, /*!< MCGIRCLK enable */
        .irclkEnableInStop = false, /*!< MCGIRCLK enable in stop mode */
        .ircs = kMcgIrcFast, /*!< Fast internal reference clock selected */
        .fcrdiv = 1U, /*!< MCG_SC[FCRDIV] */
                                
        /* -------------------- MCG FLL settings ---------------------- */
        .frdiv = 0, /*!< MCG_C1[FRDIV] */
        .drs = kMcgDcoRangeSelHigh, /*!< MCG_C4[DRST_DRS] */
        .dmx32 = kMcgDmx32Default, /*!< MCG_C4[DMX32] */
        .oscsel = kMcgOscselOsc, /*!< Selects System Oscillator (OSCCLK) */
                                
        /* -------------------- MCG PLL settings ---------------------- */
        .pll0EnableInFllMode = true, /*!< PLL0 enable in FLL mode */
        .pll0EnableInStop = false, /*!< PLL0 enable in stop mode */
        .prdiv0 = 19U, /*!< PRDIV0 */
        .vdiv0 = 24U, /*!< VDIV0 */
    },       
                        
    /*! @brief Configuration of OSCERCLK */
    .oscerConfig = 
    {
        .enable = true, /*!< OSCERCLK enable or not */
        .enableInStop = false, /*!< OSCERCLK enable or not in stop mode */
    },
                          
    /*! @brief Configuration of SIM module */  
    .simConfig = 
    {
        .pllFllSel = kClockPllFllSelPll, /*!< Pll0 clock  */
        .er32kSrc = kClockEr32kSrcRtc, /*!< ERCLK32K source selection */
        .outdiv1 = 0U, /*!< OUTDIV1 setting */
        .outdiv2 = 1U, /*!< OUTDIV2 setting */
        .outdiv3 = 2U, /*!< OUTDIV3 setting */
        .outdiv4 = 4U, /*!< OUTDIV4 setting */
    }
};    
                
/*! @brief Array of pointers to User configuration structures */
clock_manager_user_config_t const * g_clockManConfigsArr[] = {
    &clockMan1_InitConfig0,
    &clockMan1_InitConfig1,
    &clockMan1_InitConfig2,
    &clockMan1_InitConfig3,
    &clockMan1_InitConfig4
};
/*! @brief Array of pointers to User defined Callbacks configuration structures */
clock_manager_callback_user_config_t * g_clockManCallbacksArr[] = {NULL};

/* END clockMan1. */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
