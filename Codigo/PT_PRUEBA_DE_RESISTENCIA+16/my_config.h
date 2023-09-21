#ifndef __my_config
#define __my_config

#define SET(var,bit)    var |= bit
#define CLR(var,bit)    var &=~bit
#define TOGGLE(var,bit) var ^= bit
#define GET(var,bit)    (var & bit)

#define GPIO0       P0DIR,P0REN,P0OUT,P0SEL0,P0SEL1,P0IN,P0IES,P0IFG,P0IE
#define GPIO1       P1DIR,P1REN,P1OUT,P1SEL0,P1SEL1,P1IN,P1IES,P1IFG,P1IE
#define GPIO2       P2DIR,P2REN,P2OUT,P2SEL0,P2SEL1,P2IN,P2IES,P2IFG,P2IE
#define GPIO3       P3DIR,P3REN,P3OUT,P3SEL0,P3SEL1,P3IN,P3IES,P3IFG,P3IE
#define GPIO4       P4DIR,P4REN,P4OUT,P4SEL0,P4SEL1,P4IN,P4IES,P4IFG,P4IE
#define GPIO5       P5DIR,P5REN,P5OUT,P5SEL0,P5SEL1,P5IN,P5IES,P5IFG,P5IE

#define PORT_AND_PIN(n,p) BIT##p,GPIO##n

#define PIN_OUT_GET_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    GET(out,bit)
#define PIN_OUT_SET_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(out,bit)
#define PIN_OUT_CLR_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(out,bit)
#define PIN_OUT_TOGGLE_(bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    TOGGLE(out,bit)
#define PIN_IN_GET_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    GET(in, bit)
#define PIN_DIR_GET_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    GET(dir,bit)
#define PIN_DIR_OUT_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(dir,bit)
#define PIN_DIR_IN_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(dir,bit)
#define PIN_REN_GET_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    GET(ren,bit)
#define PIN_REN_EN_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(ren,bit)
#define PIN_REN_DIS_(   bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(ren,bit)
#define PIN_SEL_00_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(sel0,bit);CLR(sel1,bit)
#define PIN_SEL_01_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(sel0,bit);CLR(sel1,bit)
#define PIN_SEL_10_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(sel0,bit);SET(sel1,bit)
#define PIN_SEL_11_(    bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(sel0,bit);SET(sel1,bit)
#define PIN_IEDGE_L2H_( bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(ies,bit)
#define PIN_IEDGE_H2L_( bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(ies,bit)
#define PIN_IFLAG_CLR_( bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(ifg,bit)
#define PIN_I_EN_(      bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(ie,bit)
#define PIN_I_DIS_(     bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(ie,bit)

#define PIN_OUT_GET(pin)    PIN_OUT_GET_(pin)
#define PIN_OUT_SET(pin)    PIN_OUT_SET_(pin)
#define PIN_OUT_CLR(pin)    PIN_OUT_CLR_(pin)
#define PIN_OUT_TOGGLE(pin) PIN_OUT_TOGGLE_(pin)
#define PIN_IN_GET(pin)     PIN_IN_GET_(pin)
#define PIN_DIR_GET(pin)    PIN_DIR_GET_(pin)
#define PIN_DIR_OUT(pin)    PIN_DIR_OUT_(pin)
#define PIN_DIR_IN(pin)     PIN_DIR_IN_(pin)
#define PIN_REN_GET(pin)    PIN_REN_GET_(pin)
#define PIN_REN_EN(pin)     PIN_REN_EN_(pin)
#define PIN_REN_DIS(pin)    PIN_REN_DIS_(pin)
#define PIN_SEL_00(pin)     PIN_SEL_00_(pin)
#define PIN_SEL_01(pin)     PIN_SEL_01_(pin)
#define PIN_SEL_10(pin)     PIN_SEL_10_(pin)
#define PIN_SEL_11(pin)     PIN_SEL_11_(pin)
#define PIN_IEDGE_L2H(pin)  PIN_IEDGE_L2H_(pin)
#define PIN_IEDGE_H2L(pin)  PIN_IEDGE_L2H_(pin)
#define PIN_IFLAG_CLR(pin)  PIN_IFLAG_CLR_(pin)
#define PIN_I_EN(pin)       PIN_I_EN_(pin)
#define PIN_I_DIS(pin)      PIN_I_DIS_(pin)

#define ALL_DIR_OUT         PADIR = 0xFFFF; PBDIR = 0xFFFF; PCDIR = 0xFFFF
#define ALL_REN_EN          PAREN = 0xFFFF; PBREN = 0xFFFF; PCREN = 0xFFFF
#define ALL_OUT_CLR         PAOUT = 0x0000; PBOUT = 0x0000; PCOUT = 0x0000

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BKUP0               (*(((unsigned int *)BKMEM_BASE)+0))
#define BKUP1               (*(((unsigned int *)BKMEM_BASE)+1))
#define BKUP2               (*(((unsigned int *)BKMEM_BASE)+2))
#define BKUP3               (*(((unsigned int *)BKMEM_BASE)+3))
#define BKUP4               (*(((unsigned int *)BKMEM_BASE)+4))
#define BKUP5               (*(((unsigned int *)BKMEM_BASE)+5))
#define BKUP6               (*(((unsigned int *)BKMEM_BASE)+6))
#define BKUP7               (*(((unsigned int *)BKMEM_BASE)+7))
#define BKUP8               (*(((unsigned int *)BKMEM_BASE)+8))
#define BKUP9               (*(((unsigned int *)BKMEM_BASE)+9))
#define BKUPa               (*(((unsigned int *)BKMEM_BASE)+10))
#define BKUPb               (*(((unsigned int *)BKMEM_BASE)+11))
#define BKUPc               (*(((unsigned int *)BKMEM_BASE)+12))
#define BKUPd               (*(((unsigned int *)BKMEM_BASE)+13))
#define BKUPe               (*(((unsigned int *)BKMEM_BASE)+14))
#define BKUPf               (*(((unsigned int *)BKMEM_BASE)+15))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define base_detect_pin     PORT_AND_PIN(4,2)
#define tx_pin              PORT_AND_PIN(1,4)
#define rx_pin              PORT_AND_PIN(1,5)
#define red_pin             PORT_AND_PIN(2,4)
#define yellow_pin          PORT_AND_PIN(2,5)
#define xtl_in              PORT_AND_PIN(2,1)
#define xtl_out             PORT_AND_PIN(2,0)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BKUP_LED_STATE          BKUP0
#define BKUP_DEBOUNCING         BKUP1
#define BKUP_UART_ACTIVE        BKUP2
#define BKUP_UNCONNECTING       BKUP3
//#define BKUP_COUNT_PENDING      BKUP4
#define BKUP_AUX_UNCONNECTING   BKUP4


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FSM_START                   0
#define FSM_GETTING                 1
#define FSM_SETTING                 2
#define FSM_SETTING_RTC_ENABLE      3
#define FSM_RESETTING_0             4
#define FSM_RESETTING_1             5
#define FSM_RESETTING_2             6
#define FSM_RESETTING_3             7
#define FSM_RESETTING_NOTVALID_1    8
#define FSM_RESETTING_NOTVALID_2    9
#define FSM_RESETTING_NOTVALID_3    10

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COUNTER_CURRENT     0 //
#define COUNTER_FULL        1 //
#define COUNTER_INT_RESETTED    2 //
#define COUNTER_INT_ON_PROCESS  3  //
#define COUNTER_INT_RESETTED_ON_PROCESS 4 //
#define COUNTER_INT_FALSE_UART  5 //
#define COUNTER_INT_UART        6 //
#define COUNTER_INT_RESETTED_DURING_UART 7

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SECURE 0
#define MORE_LED 0

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MOD_CORTO (1*4-1)
#define MOD_LARGO (1023*4-1)
#define MOD_DEBOUNCE (50*4-1)//(300*4-1) // para desconeccion
#define MOD_DEBOUNCE_PARCIAL (50*4-1)
//(1023*4-1)



#define AMOUNT_OF_COUNTERS 1000//5
#define COUNTER_MAX_VALUE 65536//4
#define AUX_STIRNG_SIZE 12
#endif /* #ifndef __my_config */
