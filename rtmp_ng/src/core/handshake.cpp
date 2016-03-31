/*
 * handshake.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include "handshake.hpp"
#include "common_def.hpp"
handshake::handshake(st_netfd_t client_fd)
{
    st_net_fd = client_fd;
    step = HS_Phrase_0;
}

handshake::~handshake()
{

}

int handshake::hs_send(const void *buf, int len)
 {
     int pos = 0;
     while (pos < len){
         int written = st_write(st_net_fd, (const char *) buf + pos, len - pos, ST_UTIME_NO_TIMEOUT);
         if(written == -1 && errno == EINTR){
             _error("Interrupt by user[%x]",ERROR_INTERRUPT_BY_USR);
             return ERROR_INTERRUPT_BY_USR;
         }
         if(written == 0)
             break;
         pos += written;
     }
     return pos;
 }
