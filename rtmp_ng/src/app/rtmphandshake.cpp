/*
 * rtmphandshake.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include "rtmphandshake.hpp"
#include "rtmp_def.hpp"
#include <memory.h>

rtmp_handshake::rtmp_handshake(st_netfd_t client_fd,int init_phrase):handshake(client_fd,init_phrase)
{
    remain_data_len = 0;
}

rtmp_handshake::~rtmp_handshake()
{

}

int rtmp_handshake::process_handshake_protocol(std::string buffer,int len)
{
    if(phrase == HS_Phrase_0){
        if(len != 1 + sizeof(RTMP_HANDSHAKE))
            return ERROR_RTMP_HANDSHAKE_DATA;

        unsigned char c = buffer[0];
        memcpy(&clientsig, buffer.data() + 1, sizeof(RTMP_HANDSHAKE));
        if(c != HANDSHAKE_PLAINTEXT){
            _error("only plain text handshake supported");
            return ERROR_RTMP_HANDSHAKE_INVALID;
        }

        if(hs_send(&c, 1) < 1)
            return ERROR_RTMP_HANDSHAKE_DATA;
        memset(&serversig, 0, sizeof(RTMP_HANDSHAKE));
        serversig.flags[0] = 0x03;
        for (int i = 0; i < RANDOM_LEN; ++i)
            serversig.random[i] = rand();

        if(hs_send(&serversig, sizeof(RTMP_HANDSHAKE)) < sizeof(RTMP_HANDSHAKE))
            return ERROR_RTMP_HANDSHAKE_DATA;

        if(hs_send(&clientsig, sizeof(RTMP_HANDSHAKE)) < sizeof(RTMP_HANDSHAKE))
            return ERROR_RTMP_HANDSHAKE_DATA;
        phrase = HS_Phrase_1;
        return phrase;
    }

    if(phrase == HS_Phrase_1){
        if(len < sizeof(RTMP_HANDSHAKE))
            return ERROR_RTMP_HANDSHAKE_DATA;

        memcpy(&clientsig, buffer.data(), sizeof(RTMP_HANDSHAKE));
        if(memcmp(serversig.random, clientsig.random, RANDOM_LEN) != 0){
            _error("%s", "Final verify fail.Invalid handshake!");
            return ERROR_RTMP_HANDSHAKE_INVALID;
        }
        remain_data_len = len - sizeof(RTMP_HANDSHAKE);
        if(remain_data_len > 0){
            _trace("%s", "Remain data:");
        }
        phrase = HS_Complete;
        return phrase;
    }
    return phrase;
}

int rtmp_handshake::do_handshake()
{
    return RESULT_SUCCESS;
}

int rtmp_handshake::get_remain_data_len()
{
    return remain_data_len;
}

const char * rtmp_handshake::get_remain_data_ptr()
{
    return NULL;
}
