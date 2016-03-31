/*
 * server.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include "server.hpp"
#include<memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdexcept>

server::server()
{
    max_thread = cpu_count() * 8;
    server_net_fd = NULL;
    current_thread = 0;
}

server::~server()
{

}

int server::init_server(char * ip,int port,bool mode)
{
    int len;

    st_set_eventsys(ST_EVENTSYS_ALT);

    if(st_init() < 0){
        _error("Initialization failed:[%x]",ERROR_ST_INITIALIZE);
        return ERROR_ST_INITIALIZE;
    }

    _info("System event :[ %s ]", st_get_eventsys_name());
    st_timecache_set(1);

    if((len = strlen(ip))>256){
        _error("IP value invalid:[%x]",ERROR_PARAMETER_INVALID_IP);
        return ERROR_PARAMETER_INVALID_IP;
    }

    strcpy(ip_address,ip);
    is_tcp = mode;
    if(is_tcp)
        tcp_port = port;
    else
        udp_port = port;

    return RESULT_SUCCESS;
}

void server::start_server()
{
    try{
        create_listener();
    }catch(const std::runtime_error &e){
        _error("Create Listener catch exception: %s", e.what());
        exit(RESULT_ERROR);
    }
}

int server::create_listener(void)
 {
     int n, sock;
     struct sockaddr_in serv_addr;
     struct hostent *hp;

     if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
         _error("[%x]Can't create socket,ERRNO",errno);
         throw std::runtime_error("Can't create socket");
     }
     n = 1;
     if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) < 0){
         _error("[%x]Can't set SO_REUSEADDR,ERRNO",errno);
         throw std::runtime_error("Can't set SO_REUSEADDR");
     }

     memset(&serv_addr, 0, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_port = htons(tcp_port);
     serv_addr.sin_addr.s_addr = inet_addr(ip_address);
     if(serv_addr.sin_addr.s_addr == INADDR_NONE){
         if((hp = gethostbyname(ip_address)) == NULL){
             _error("[%x]Can't resolve address: %s", ERROR_NET_LOOKUP,ip_address);
             throw std::runtime_error("Can't lookup");
         }
         memcpy(&serv_addr.sin_addr, hp->h_addr, hp->h_length);
     }

     if(bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
         _error("[%x]Can't bind to address %s:%d", ERROR_NET_BIND,ip_address, tcp_port);
         throw std::runtime_error("Can't bind to address");
     }

     if(listen(sock, 10) < 0){
         _error("[%x]Can't listen at %d",ERROR_NET_LISTEN,tcp_port);
         throw std::runtime_error("Can't bind to address");
     }

     server_net_fd = st_netfd_open_socket(sock);
     if(server_net_fd == NULL){
         _error("[%x]Can't open st netfd,ERRNO",ERROR_ST_NETFD_OPEN);
         throw std::runtime_error("Can't open st netfd");
     }

     return RESULT_SUCCESS;
 }

int server::cpu_count()
{
    int n;
#if defined (_SC_NPROCESSORS_ONLN)
    n = (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined (_SC_NPROC_ONLN)
    n = (int) sysconf(_SC_NPROC_ONLN);
#elif defined (HPUX)
#include <sys/mpctl.h>
    n = mpctl(MPC_GETNUMSPUS, 0, 0);
#else
    n = -1;
    errno = ENOSYS;
#endif
    return n;
}
