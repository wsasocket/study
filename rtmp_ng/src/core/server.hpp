/*
 * server.hpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#ifndef SRC_SERVER_HPP_
#define SRC_SERVER_HPP_

#include <st.h>
#include <vector>
#include <stdio.h>
#include "common_def.hpp"
#include "protocol.hpp"

/*
 * server class implement all process before accept
 */
class server
{
public:
    server();
    virtual ~server();

public:
    st_netfd_t server_net_fd;
    int max_thread;
    int current_thread;

public:
    int init_server(char *ip, int port, bool mode);
    void start_server();

private:
    int create_listener();
    int cpu_count();

private:
    char ip_address[256];
    int tcp_port;
    int udp_port;
    bool is_tcp;
};

#endif /* SRC_SERVER_HPP_ */
