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
    st_net_fd = client_fd;
    handshake_ptr = NULL;
}

protocol::~protocol()
{
    if(handshake_ptr != NULL)
        delete handshake_ptr;
}

void protocol::do_poll()
{
    try{
        while (handshake_ptr->phrase != HS_Complete)
            handshake_ptr->do_handshake();
    } catch (const std::runtime_error &e){
        _error("Handshake protocol catch exception: %s", e.what());
        return;
    }

    try{
        parse_protocol();
    } catch (const std::runtime_error &e){
        _error("Protocol catch exception: %s", e.what());
        return;
    }
}
