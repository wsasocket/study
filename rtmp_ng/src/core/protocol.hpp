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
 * if derived protocol need hanshake
 * should new handshake class and set_handshake()
 * some protocol like echo can be set NULL
 */
class protocol
{
public:
    protocol(st_netfd_t client_fd);
    virtual ~protocol();

public:
    void do_poll() ;
    void close_connect();
    virtual void parse_protocol() = 0;
    void set_handshake(handshake * hs_ptr);

public:
    st_netfd_t st_net_fd;
    bool is_handshake_complete;
    handshake *handshake_ptr;
    std::string buf;
    std::string send_queue;
};

#endif /* SRC_PROTOCOL_HPP_ */
