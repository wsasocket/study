/*
 * rtmphandshake.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include "rtmphandshake.hpp"
#include "rtmp_def.hpp"
#include <arpa/inet.h>
#include <stdexcept>
#include <memory.h>

rtmp_handshake::rtmp_handshake(st_netfd_t client_fd, int init_phrase) :
        handshake(client_fd, init_phrase)
{

}

rtmp_handshake::~rtmp_handshake()
{
}

int rtmp_handshake::do_handshake()
{
    std::string recv_buffer(sizeof(RTMP_HANDSHAKE) + 10, 0);
    struct in_addr *from = (struct in_addr *) st_netfd_getspecific(st_net_fd);
    ssize_t read_size;
    // HS_Phrase_0;
    while (1){
        read_size = st_read_fully(st_net_fd, &recv_buffer[0], 1 + sizeof(RTMP_HANDSHAKE), SEC2USEC(REQUEST_TIMEOUT));
        if(read_size == -1 && errno == ETIME){
            continue;
        }
        break;
    }
    if(read_size == 0){
        _error("Network connection from %s is closed.", inet_ntoa(*from));
        throw std::runtime_error("Network connection lost!");
    }

    //parse section
    if(read_size != 1 + sizeof(RTMP_HANDSHAKE)){
        _error("C0+C1 = %d",read_size);
        throw std::runtime_error("Handshake data overflow at phrase1");
    }
    unsigned char c = recv_buffer[0];
    memcpy(&clientsig, recv_buffer.data() + 1, sizeof(RTMP_HANDSHAKE));
    if(c != HANDSHAKE_PLAINTEXT){
        _error("only plain text handshake supported");
        throw std::runtime_error("Handshake protocol unsupport!");
    }

    if(hs_send(&c, 1) < 1)
        throw std::runtime_error("Handshake protocol send function fail!");
    memset(&serversig, 0, sizeof(RTMP_HANDSHAKE));
    serversig.flags[0] = 0x03;
    for (int i = 0; i < RANDOM_LEN; ++i)
        serversig.random[i] = rand();

    if(hs_send(&serversig, sizeof(RTMP_HANDSHAKE)) < sizeof(RTMP_HANDSHAKE))
        throw std::runtime_error("Handshake protocol send function fail!");

    if(hs_send(&clientsig, sizeof(RTMP_HANDSHAKE)) < sizeof(RTMP_HANDSHAKE))
        throw std::runtime_error("Handshake protocol send function fail!");

//    phrase = HS_Phrase_1;
    while (1){
        read_size = st_read_fully(st_net_fd, &recv_buffer[0], sizeof(RTMP_HANDSHAKE), SEC2USEC(REQUEST_TIMEOUT));
        if(read_size == -1 && errno == ETIME){
            continue;
        }
        break;
    }
    if(read_size == 0){
        _error("Network connection from %s is closed.", inet_ntoa(*from));
        throw std::runtime_error("Network connection lost!");
    }

    if(read_size < sizeof(RTMP_HANDSHAKE))
        throw std::runtime_error("Handshake data overflow at phrase2");

    memcpy(&clientsig, recv_buffer.data(), sizeof(RTMP_HANDSHAKE));
    if(memcmp(serversig.random, clientsig.random, RANDOM_LEN) != 0){
        _error("%s", "Final verify fail.Invalid handshake!");
        throw std::runtime_error("Handshake protocol verify fail!");
    }
//    HS_Complete;
    phrase = HS_Complete;
    return HS_Complete;
}

