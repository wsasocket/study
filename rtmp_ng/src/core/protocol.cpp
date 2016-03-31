/*
 * protocol.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include "protocol.hpp"
#include <st.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../3rd/utils.h"

protocol::protocol(st_netfd_t client_fd)
{
    is_handshake_complete = true;
    st_net_fd = client_fd;
    handshake_ptr = NULL;
}

protocol::~protocol()
{

}
void protocol::close_connect()
{
    if(handshake_ptr != NULL)
        delete handshake_ptr;
    st_netfd_close(st_net_fd);
    st_netfd_free(st_net_fd);
}

void protocol::set_handshake(handshake * hs_ptr)
{
    if(hs_ptr == NULL)
        return;
    is_handshake_complete = false;
    handshake_ptr = hs_ptr;
}

void protocol::do_poll()
{
    std::string recv_buffer(MAX_BUF_LEN, 0);
    struct in_addr *from = (struct in_addr *) st_netfd_getspecific(st_net_fd);
    ssize_t read_size;

    while (true){
        read_size = st_read(st_net_fd, &recv_buffer[0], MAX_BUF_LEN, SEC2USEC(REQUEST_TIMEOUT));
        if(read_size == -1 && errno == ETIME){
            continue;
        }
        if(read_size == 0){
            _error("Network connection from %s is closed.", inet_ntoa(*from));
            return;
        }

        if(!is_handshake_complete){
            try{
                handshake_ptr->process_handshake_protocol(recv_buffer, read_size);
                if(handshake_ptr->step == HS_Complete){
                    is_handshake_complete = true;
                    int remain_size = handshake_ptr->get_remain_data_len();
                    if(remain_size > 0){
                        buf.clear();
                        buf.append(recv_buffer.data(), 0,remain_size);
#ifdef __DEBUG__
                        _trace("Data remain %x:%x",remain_size,buf.size());
                        ::hexdump(buf.data(),remain_size);
#endif
                    }
                } else continue;
            } catch (const std::runtime_error &e){
                _error("Handshake protocol catch exception: %s", e.what());
                return;
            }
        } else buf.append(recv_buffer, 0, read_size);
        try{
            parse_protocol();
        } catch (const std::runtime_error &e){
            _error("Protocol catch exception: %s", e.what());
            return;
        }
    }
}
