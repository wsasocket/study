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
    rtmp * client_ptr = (rtmp *) param;
    client_ptr->do_poll();
    client_ptr->close_connect();
    client_ptr->server_ptr->cleanup(client_ptr);
    return nullptr;
}

void rtmp_server::start_thread()
{
    st_netfd_t cli_nfd;
    struct sockaddr_in from;
    int fromlen = sizeof(from);;

    while (1){
         cli_nfd = st_accept(server_net_fd, (struct sockaddr *) &from, &fromlen, ST_UTIME_NO_TIMEOUT);
         if(cli_nfd == NULL){
             _error("Can't accept connection: st_accept [%x]",ERROR_ST_ACCEPT);
             return ;
         }
         st_netfd_setspecific(cli_nfd, &from.sin_addr, NULL);

         if(current_thread > max_thread){
             _error("Exceed system limit:[%x]",ERROR_ST_EXCEED_LIMIT);
             return;
         }
         rtmp * client = new rtmp (cli_nfd,this);
         if(st_thread_create(rtmp_thread_func, (void *) client, 0, 0) == NULL){
             _error("Process can't create thread [%x]",ERROR_ST_CREATE_THREAD);
             return ;
         }
         current_thread ++;
         client_list.push_back(client);
         _info("Total Client:[%d] Current [%x]", current_thread, cli_nfd);
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
        current_thread --;
    }
    _trace("Client instance left %d", current_thread);
}
