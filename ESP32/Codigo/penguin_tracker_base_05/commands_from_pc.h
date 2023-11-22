#ifndef __commands_from_pc_h__
#define __commands_from_pc_h__

#define CONNECT_TO_MSP  'S' 
#define GET_FSM_STATE   'F' //
#define GET_LOGGER_ID   'I' //
#define GET_NAME        'N' //
#define SET_NAME        'n' //
#define GET_WET_AND_DRY_FREC  'W'  //
#define SET_WET_AND_DRY_FREC  'w'  //
#define GET_TEMPERATURE_FREC  'T'  //
#define SET_TEMPERATURE_FREC  't'  //
#define GET_OPERATING_MODE  'M'   
#define SET_OPRETAING_MODE  'm'
#define OPERATING_MODE__INACTIVE    '0'
#define OPERATING_MODE__WET_AND_DRY '1'
#define OPERATING_MODE__TEMPERATURE '2'
#define OPERATING_MODE__BOTH        '3'
#define GET_CALIBRATED_MINUTE 'C' //
#define SET_CALIBRATED_MINUTE 'c' //
#define GET_ACTIVATION_TIME   'A' //
#define SET_ACTIVATION_TIME   'a' 
#define GET_AT_30C            '3' //
#define GET_AT_105C           '1' //
#define ACTIVATE_LOGGER       'Q'
#define DEACTIVATE_LOGGER     'q'
#define TOGGLE_LED            'L' //
#define GET_DATA_FAST         'D' 
#define GET_EVENTS            'E' //
#define GET_ALL_INDEX         'X' //

#define MASTER_CONTROL_COMMAND  '~' //


//13AaDECcFILMmNnQqSTtWwX~

#endif