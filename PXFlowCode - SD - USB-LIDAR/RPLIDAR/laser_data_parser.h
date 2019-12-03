#ifndef LASER_DATA_PARSER_H
#define LASER_DATA_PARSER_H
#include "laser_parse_define.h"
#include "string.h"

#define Check_First_Byte ==LASER_DATA_FIRSTBYTE
#define Check_Second_Byte ==LASER_DATA_FIRSTBYTE




    uint16_t higher_byte(uint8_t c);

    void laser_data_start_checksum(laser_data_message_t* msg);
    void laser_data_update_checksum(laser_data_message_t* msg, uint16_t input_byte);
    /***********Parse new recieved byte and update state machine*************/
    uint8_t laser_data_packet_char_buffer(laser_data_message_t* rxmsg,
                                          laser_data_status_t* status,
                                          uint8_t c,
                                          laser_data_message_t* r_message,
                                          laser_data_status_t* r_data_status);

    uint8_t laser_data_packet_char(uint8_t input_byte,
                                   laser_data_message_t* r_message,
                                   laser_data_status_t* r_mavlink_status);

    /*******************Parse new recieved byte*****************************/
    uint8_t laser_data_parse_char(uint8_t input_byte,
                                  laser_data_message_t* r_message,
                                  laser_data_status_t* r_status);




#endif // LASER_DATA_PARSER_H
