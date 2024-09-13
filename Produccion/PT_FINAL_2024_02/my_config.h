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
#define PIN_IFLAG_SET_( bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(ifg,bit)
#define PIN_IFLAG_GET_( bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    GET(ifg,bit)
#define PIN_I_EN_(      bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    SET(ie,bit)
#define PIN_I_DIS_(     bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    CLR(ie,bit)
#define PIN_I_GET_(     bit,dir,ren,out,sel0,sel1,in,ies,ifg,ie,...)    GET(ie,bit)

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
#define PIN_IFLAG_SET(pin)  PIN_IFLAG_SET_(pin)
#define PIN_IFLAG_GET(pin)  PIN_IFLAG_GET_(pin)
#define PIN_I_EN(pin)       PIN_I_EN_(pin)
#define PIN_I_DIS(pin)      PIN_I_DIS_(pin)
#define PIN_I_GET(pin)      PIN_I_GET_(pin)

#define ALL_DIR_OUT         PADIR = 0xFFFF; PBDIR = 0xFFFF; PCDIR = 0xFFFF
#define ALL_REN_EN          PAREN = 0xFFFF; PBREN = 0xFFFF; PCREN = 0xFFFF
#define ALL_OUT_CLR         PAOUT = 0x0000; PBOUT = 0x0000; PCOUT = 0x0000

#define WRITE_FRAM_ENABLE           SYSCFG0 = FRWPPW | DFWP
#define WRITE_DFRAM_ENABLE          SYSCFG0 = FRWPPW | PFWP
#define WRITE_FRAM_DISABLE          SYSCFG0 = FRWPPW | PFWP | DFWP

#define UPDATE_PERSISTENT_VAR(var,value) {\
    WRITE_FRAM_ENABLE; \
    (var) = (value); \
    WRITE_FRAM_DISABLE;}

#define LENGTH(array)   (sizeof(array)/sizeof((array)[0]))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define BKUP0               (*(((volatile unsigned int *)BKMEM_BASE)+0))
#define BKUP1               (*(((volatile unsigned int *)BKMEM_BASE)+1))
#define BKUP2               (*(((volatile unsigned int *)BKMEM_BASE)+2))
#define BKUP3               (*(((volatile unsigned int *)BKMEM_BASE)+3))
#define BKUP4               (*(((volatile unsigned int *)BKMEM_BASE)+4))
#define BKUP5               (*(((volatile unsigned int *)BKMEM_BASE)+5))
#define BKUP6               (*(((volatile unsigned int *)BKMEM_BASE)+6))
#define BKUP7               (*(((volatile unsigned int *)BKMEM_BASE)+7))
#define BKUP8               (*(((volatile unsigned int *)BKMEM_BASE)+8))
#define BKUP9               (*(((volatile unsigned int *)BKMEM_BASE)+9))
#define BKUPa               (*(((volatile unsigned int *)BKMEM_BASE)+10))
#define BKUPb               (*(((volatile unsigned int *)BKMEM_BASE)+11))
#define BKUPc               (*(((volatile unsigned int *)BKMEM_BASE)+12))
#define BKUPd               (*(((volatile unsigned int *)BKMEM_BASE)+13))
#define BKUPe               (*(((volatile unsigned int *)BKMEM_BASE)+14))
#define BKUPf               (*(((volatile unsigned int *)BKMEM_BASE)+15))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// max 10 min:
//#define SET_RTC_TO_SECS(s)          RTCMOD = ((unsigned int)((s)*100))-1; RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__100 | RTCIE
//#define SET_REMAINING_RTC_CNT(cnt)  RTCMOD = cnt; RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__100 | RTCIE
// max 6 sec:
//#define SET_RTC_TO_MSECS(s)         RTCMOD = ((unsigned int)((s)*10))-1; RTCCTL = RTCSS__VLOCLK | RTCSR | RTCPS__1 | RTCIE

#define DISABLE_RTC                   RTCCTL &= ~RTCIE

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define ADC_GAIN            (*((unsigned short *)0x1A16))
#define ADC_OFFSET          (*((unsigned short *)0x1A18))

#define CALADC_15V_30C      *((unsigned int *)0x1A1A)                 // Temperature Sensor Calibration-30 C
                                                                  // See device datasheet for TLV table memory mapping
#define CALADC_15V_105C     *((unsigned int *)0x1A1C)                 // Temperature Sensor Calibration-105 C

/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif /* #ifndef __my_config */
