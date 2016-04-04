/*
 * protocol.hpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#ifndef SRC_PROTOCOL_HPP_
#define SRC_PROTOCOL_HPP_

#include <string>
#include <st.h>
#include"handshake.hpp"
#include"common_def.hpp"

class server;
/*
 * This class is base class of protocol
 * if derived protocol need handshake
 * some protocol like echo has not handshake
 * set init_phrase = HS_Complete
 */
class protocol
{
public:
    protocol(st_netfd_t client_fd);
    virtual ~protocol();

public:
    void do_poll() ;
    virtual void close_connect() = 0;
    virtual void parse_protocol() = 0;
    virtual void parse_handshake_protocol() = 0;
    virtual handshake * create_handshake(int init_phrase) = 0;

public:
    st_netfd_t st_net_fd;
    handshake *handshake_ptr;
    std::string buf;
    std::string send_queue;
};

#endif /* SRC_PROTOCOL_HPP_ */
