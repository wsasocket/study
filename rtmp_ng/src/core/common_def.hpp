/*
 * common_def.hpp
 *
 *  Created on: 2016年3月30日
 *      Author: james
 */

#ifndef SRC_CORE_COMMON_DEF_HPP_
#define SRC_CORE_COMMON_DEF_HPP_

#include <stdio.h>
#include<stdint.h>

#define PACKED  __attribute__((packed))

#define _trace(fmt...) fprintf(stderr,"[-]");\
        fprintf(stderr,fmt);\
        fprintf(stderr,"\n")

#define _error(fmt...) fprintf(stderr,"[x][%s]:[%d]:[%s] --> ",__FILE__,__LINE__,__FUNCTION__);\
        fprintf(stderr,fmt);\
        fprintf(stderr,"\n")

#define _info(fmt...) fprintf(stderr,"[I]---> ");\
        fprintf(stderr,fmt);\
        fprintf(stderr,"<---\n")
#define SEC2USEC(s) ((s)*1000000LL)
//------------common use parameter
static const int MAX_BUF_LEN = 4096 + 32;
static const int REQUEST_TIMEOUT = 10;
//--------------error constant defined ----------
static const int RESULT_SUCCESS = 1;
static const int RESULT_ERROR = 0;
static const int ERROR_SERVER_CREATE = 0x100;
static const int ERROR_ST_INITIALIZE = 0x101;
static const int ERROR_PARAMETER_INVALID_IP = 0x102;
static const int ERROR_NET_LOOKUP = 0x103;
static const int ERROR_NET_BIND = 0x104;
static const int ERROR_NET_LISTEN = 0x105;
static const int ERROR_ST_NETFD_OPEN = 0x106;
static const int ERROR_INTERRUPT_BY_USR = 0x107;
static const int ERROR_ST_EXCEED_LIMIT = 0x108;
static const int ERROR_ST_CREATE_THREAD = 0x109;
static const int ERROR_ST_ACCEPT = 0x10a;
#endif /* SRC_CORE_COMMON_DEF_HPP_ */
