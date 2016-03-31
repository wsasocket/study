/*
 * rtmpserver.h
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#ifndef SRC_RTMPSERVER_HPP_
#define SRC_RTMPSERVER_HPP_

#include "../core/server.hpp"
#include<vector>

class rtmp;
/* This class is derived from class server
 *  in rtmp protocol ,publisher is different from other player
 *  client_list is used to save connected objects
*/
class rtmp_server: public server
{
public:
    rtmp_server();
    virtual ~rtmp_server();

public:
    void start_thread();
    friend void *rtmp_thread_func (void * param);
    void cleanup(protocol * client_ptr);

public:
    rtmp * publisher;
    std::vector<rtmp* > client_list;
};

#endif /* SRC_RTMPSERVER_HPP_ */
