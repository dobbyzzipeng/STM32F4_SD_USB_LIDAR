#include "laser_data_parser.h"
laser_data_status_t laser_data_status_buffer;
laser_data_message_t laser_data_message_buffer;

void Laser_Data_Parser(void)
{
	laser_data_status_buffer.packet_idx=0;
	laser_data_status_buffer.data_sub_state=LASER_DATA_FIRSTBYTE;
	laser_data_status_buffer.parse_state = LASER_DATA_PARSE_STATE_IDLE;
	laser_data_status_buffer.parse_error=0;
	laser_data_status_buffer.packet_rx_success_count=0;
	laser_data_status_buffer.packet_rx_drop_count=0;
	laser_data_status_buffer.msg_received=LASER_DATA_PACKET_INCOMPLETE;
}

uint16_t higher_byte(uint8_t input_byte){
	uint16_t hb=(uint16_t)input_byte;
	hb=hb*256;
	return hb;
}


void laser_data_start_checksum(laser_data_message_t* msg)
{
	msg->checksum=0;
}

void laser_data_update_checksum(laser_data_message_t* msg, uint16_t input_byte)
{
	msg->checksum=msg->checksum+input_byte;
}



uint8_t laser_data_parse_char(uint8_t input_byte,
		laser_data_message_t* r_message,
		laser_data_status_t* r_status)
{
	uint8_t msg_received = laser_data_packet_char(input_byte, r_message, r_status);
	if (msg_received == LASER_DATA_PACKET_BAD_CHECK_SUM) {
		// we got a bad check sum. Treat as a parse failure
		laser_data_status_buffer.parse_error++;
	}
	return msg_received;
}


uint8_t laser_data_packet_char(uint8_t input_byte,
		laser_data_message_t* r_message,
		laser_data_status_t* r_status)
{
	return laser_data_packet_char_buffer(&laser_data_message_buffer,
			&laser_data_status_buffer,
			input_byte,
			r_message,
			r_status);
}



uint8_t laser_data_packet_char_buffer(laser_data_message_t* rxmsg,
		laser_data_status_t* status,
		uint8_t c,
		laser_data_message_t* r_message,
		laser_data_status_t* r_data_status)
{


//	std::cout<<"AAA"<<std::endl;
	status->msg_received = LASER_DATA_PACKET_INCOMPLETE;

	switch (status->parse_state)
	{
		case LASER_DATA_PARSE_STATE_IDLE://Idle state wait for start flag
			if(status->data_sub_state==LASER_DATA_FIRSTBYTE)
			{
				if (c == PACKAGE_STX_FIRST)
				{
					status->data_sub_state=LASER_DATA_SECONDBYTE;
				}
			}
			else
			{
				if(c == PACKAGE_STX_SECOND)
				{
					status->parse_state = LASER_DATA_PARSE_STATE_GOT_STX;

					status->packet_idx = 0;
					rxmsg->len = 0;
					rxmsg->begin_angle = 0;
					laser_data_start_checksum(rxmsg);
				//	std::cout<<"Got Head"<<std::endl;
				}
				status->data_sub_state = LASER_DATA_FIRSTBYTE;

			}
			break;

		case LASER_DATA_PARSE_STATE_GOT_STX://Get start flag wait for data length
			if(status->data_sub_state==LASER_DATA_FIRSTBYTE)
			{
				rxmsg->len=c;
				laser_data_update_checksum(rxmsg,c);
				status->data_sub_state=LASER_DATA_SECONDBYTE;

			}
			else
			{
				rxmsg->len=rxmsg->len+higher_byte(c);
                		laser_data_update_checksum(rxmsg,higher_byte(c));
				status->data_sub_state = LASER_DATA_FIRSTBYTE;
				status->parse_state =LASER_DATA_PARSE_STATE_GOT_LENGTH;
			//	std::cout<<"length"<<rxmsg->len<<std::endl;
			}
			break;

		case LASER_DATA_PARSE_STATE_GOT_LENGTH://Get data length wait for begin angle
			if(status->data_sub_state==LASER_DATA_FIRSTBYTE)
			{
				laser_data_update_checksum(rxmsg,c);
				status->data_sub_state=LASER_DATA_SECONDBYTE;
				rxmsg->begin_angle=c;
			}
			else
			{
				laser_data_update_checksum(rxmsg,higher_byte(c));
				status->data_sub_state = LASER_DATA_FIRSTBYTE;
				rxmsg->begin_angle=rxmsg->begin_angle+higher_byte(c);
                status->parse_state =LASER_DATA_PARSE_STATE_GOT_BEGIN_ANGLE;
			}
			break;
		case LASER_DATA_PARSE_STATE_GOT_BEGIN_ANGLE://Get begin angle wait for all data;

			if(status->data_sub_state==LASER_DATA_FIRSTBYTE)
			{
				laser_data_update_checksum(rxmsg,c);
				status->data_sub_state=LASER_DATA_SECONDBYTE;
				rxmsg->distance[status->packet_idx]=c;
			}
			else
			{
				rxmsg->distance[status->packet_idx]=rxmsg->distance[status->packet_idx]+higher_byte(c);
				status->packet_idx++;
				laser_data_update_checksum(rxmsg,higher_byte(c));
				status->data_sub_state = LASER_DATA_FIRSTBYTE;
				if(status->packet_idx==rxmsg->len)status->parse_state =LASER_DATA_PARSE_STATE_GOT_ALL_DATA;;
			}
			break;
		case LASER_DATA_PARSE_STATE_GOT_ALL_DATA:
			if(status->data_sub_state==LASER_DATA_FIRSTBYTE)
			{
				status->data_sub_state=LASER_DATA_SECONDBYTE;
				rxmsg->checksum_ack=c;
			}
			else
			{
				rxmsg->checksum_ack=rxmsg->checksum_ack+higher_byte(c);
				status->data_sub_state = LASER_DATA_FIRSTBYTE;
				if(rxmsg->checksum_ack!=rxmsg->checksum){
				//	std::cout<<"checksum_ack"<<rxmsg->checksum_ack<<"rx_checksum"<<rxmsg->checksum<<std::endl;
					status->msg_received=LASER_DATA_PACKET_BAD_CHECK_SUM;
				}
				else
				{
					status->msg_received=LASER_DATA_PACKET_OK;

				}
				status->parse_state=LASER_DATA_PARSE_STATE_IDLE;
				memcpy(r_message, rxmsg, sizeof(laser_data_message_t));
			}
			break;
	}
	/*************************End of State Machine************************/

	if (status->msg_received == LASER_DATA_PACKET_OK)
	{

		// Initial condition: If no packet has been received so far, drop count is undefined
		if (status->packet_rx_success_count == 0) status->packet_rx_drop_count = 0;
		// Count this packet as received
		status->packet_rx_success_count++;
	}

	r_message->len = rxmsg->len; // Provide visibility on how far we are into current msg
	r_data_status->parse_error= status->parse_error;
	r_data_status->parse_state = status->parse_state;
	r_data_status->packet_idx = status->packet_idx;
	r_data_status->packet_rx_success_count = status->packet_rx_success_count;
	r_data_status->packet_rx_drop_count = status->parse_error;

	return status->msg_received;
}

