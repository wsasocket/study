/*
 * rtmpserver.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include "rtmpserver.hpp"
#include "rtmp.hpp"
#include<memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdexcept>
#include <st.h>

rtmp_server::rtmp_server()
{
    publisher = NULL;
    client_list.clear();
}

rtmp_server::~rtmp_server()
{
    std::vector<rtmp *> ::iterator i;
    while(client_list.size()!=0){
        i = client_list.begin() ;
        protocol * client = *i;
        st_netfd_close(client->st_net_fd);
        st_netfd_free(client->st_net_fd);
        client_list.erase(i);
        delete client;
    }
    _info("RTMP server shutdown");
}

void *rtmp_thread_func (void * param)
{
    st_netfd_t cli_nfd;
    struct sockaddr_in from;
    int fromlen = sizeof(from);
    rtmp_server * _server = (rtmp_server * ) param;

    while (1){
         cli_nfd = st_accept(_server->server_net_fd, (struct sockaddr *) &from, &fromlen, ST_UTIME_NO_TIMEOUT);
         if(cli_nfd == NULL){
             _error("Can't accept connection: st_accept [%x]",ERROR_ST_ACCEPT);
             _server->wait_thread --;
             return nullptr;
         }
         _server->busy_thread ++;
         _server->wait_thread --;
         st_netfd_setspecific(cli_nfd, &from.sin_addr, NULL);
         rtmp * client = new rtmp (cli_nfd,_server);
         _server->client_list.push_back(client);
         _trace("[Connect] Current wait:[%d] busy:[%d]", _server->wait_thread,_server->busy_thread);
         client->do_poll();
         client->close_connect();
         _server->cleanup(client);
         _server->busy_thread --;
         _server->wait_thread ++;
         _trace("[Disconnect]Current wait:[%d] busy:[%d]", _server->wait_thread,_server->busy_thread);
         }
}


void rtmp_server::start_thread()
{
    int limit = 0;
    while(1){
        if(wait_thread > 1){
            st_usleep(1000);
            continue;
        }
        limit = (wait_thread + busy_thread +2 < max_thread)?2:(max_thread - wait_thread - busy_thread);
        for (int i =0;i< limit;i++){
             if(st_thread_create(rtmp_thread_func, (void *)this, 0, 0) == NULL){
                 _error("Process can't create thread [%x]",ERROR_ST_CREATE_THREAD);
                 return ;
             }
             wait_thread ++;
        }
        _info("Total Thread [%d] created", wait_thread + busy_thread);
        st_usleep(1000);
    }
}

void rtmp_server::cleanup(protocol * client_ptr)
{
    rtmp * f = NULL;
    bool can_be_release = false;
    std::vector<rtmp *>::iterator i;
    for (i = client_list.begin(); i != client_list.end(); i++){
        f = *i;
        if(client_ptr == f){
            can_be_release = true;
            break;
        }
    }
    if(can_be_release){
        client_list.erase(i);
        if(f == publisher){
            _trace("%s", "Publisher disconnect,Notify all player");
            std::vector<rtmp *>::iterator i;
            rtmp *player;
            for (i = client_list.begin(); i != client_list.end(); i++){
                player = *i;
                player->ready = false;
            }
            publisher = NULL;
        }
        delete client_ptr;
        client_ptr = NULL;
    }
}
