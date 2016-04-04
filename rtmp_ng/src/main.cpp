/*
 * main.cpp
 *
 *  Created on: 2016年3月30日
 *      Author: james
 */
#include "app/rtmpserver.hpp"

void usage(char * filename)
{
    printf("Usage:\n%s [port]\n", filename);
    printf("Default value is RTMP protocol port 1935\n");
}

int main(int argc, char ** argv)
{
    char ip[] = "0.0.0.0";
    int port = 1935;

    if(argc != 2){
        usage(argv[0]);
        printf("Use default value to start RTMP server\n");
    } else{
        port = atol(argv[1]);
        if(port < 1024 || port > 65535){
            printf("Port value [%d] is invalid,use 1025~65534\n", port);
            return RESULT_ERROR;
        }
        printf("RTMP Server will listen port %d\n", port);
    }

    rtmp_server server;
    server.init_server(ip, port, true);
    server.start_server();
    server.start_thread();
    return RESULT_SUCCESS;
}

