/*
 * rtmphandshake.hpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#ifndef SRC_RTMPHANDSHAKE_HPP_
#define SRC_RTMPHANDSHAKE_HPP_

#include "../core/handshake.hpp"
#include "rtmp_def.hpp"
#include "../core/common_def.hpp"

/* This class is derived from class handshake
 * we should implement process_handshake_protocol()
*  sometimes received data are more than handshake protocol
*  need.We MUST save remained data for further use.
*/
class rtmp_handshake:public handshake
{
public:
    rtmp_handshake(st_netfd_t client_fd);
    virtual ~rtmp_handshake();

public:
    int process_handshake_protocol(std::string buf,int len) ;
    int get_remain_data_len();

private:
    RTMP_HANDSHAKE serversig;
    RTMP_HANDSHAKE clientsig;
    int remain_data_len;
};

#endif /* SRC_RTMPHANDSHAKE_HPP_ */
