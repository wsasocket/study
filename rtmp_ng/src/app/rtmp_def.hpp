/*
 * newdef.hpp
 *
 *  Created on: 2016年3月30日
 *      Author: james
 */
#ifndef SRC_APP_NEWDEF_HPP_
#define SRC_APP_NEWDEF_HPP_

#include"../core/common_def.hpp"

//---------rtmp error code define--------------------------
static const int ERROR_RTMP_HANDSHAKE_DATA = 0x200;
static const int ERROR_RTMP_HANDSHAKE_INVALID = 0x201;
static const int ERROR_RTMP_ST_NET_WRITE = 0x202;
static const int ERROR_RTMP_ST_NET_READ = 0x203;
static const int ERROR_RTMP_UNSUPPORT_APP = 0x204;
static const int ERROR_RTMP_PARSE = 0x205;
//---------rtmp protocol variable --------------------------
static const int RANDOM_LEN = 1536 - 8;
static const uint8_t HANDSHAKE_PLAINTEXT = 0x03;
static const int DEFAULT_CHUNK_LEN = 128;
//---------rtmp protocol constant define ---------------
#define MSG_SET_CHUNK       0x01      //payload is 32bit first bit must be zero
#define MSG_ABORT   0x02                  /payload 32bit is chunk stream id
#define MSG_ACK     0x03                     //payload 32 sequence number
#define MSG_USER_CONTROL    0x04  //payload 16bit event type and event
#define MSG_WINDOW_SIZE     0x05   //poayload 32bit
#define MSG_SET_PEER_WINDOW_SIZE        0x06//payload 32bit window size +8bit type 0,1,2
#define MSG_AUDIO       0x08
#define MSG_VIDEO       0x09
#define MSG_NOTIFY_AMF3 0x0f;
#define MSG_OBJECT_AMF3     0x10
#define MSG_INVOKE_AMF3     0x11    // AMF3
#define MSG_NOTIFY_AMF0     0x12      //meta data about some thing
#define MSG_OBJECT_AMF0     0x13
#define MSG_INVOKE_AMF0     0x14    // AMF0
#define MSG_AGGREGATE_MSG       0x16

#define CONTROL_STREAM_BEGIN    0x00
#define CONTROL_STREAM_EOF  0x01
#define CONTROL_STREAM_DRY  0x02
#define CONTROL_BUFFER_TIME 0x03
#define CONTROL_STREAM_REC 0x04
#define CONTROL_PING    0x06
#define CONTROL_PONG        0x07
#define CONTROL_REQUEST_VERIFY  0x1a
#define CONTROL_RESPOND_VERIFY  0x1b
#define CONTROL_BUFFER_EMPTY    0x1f
#define CONTROL_BUFFER_READY    0x20

#define CONTROL_ID      0
#define STREAM_ID       1337
#define CHAN_CONTROL        2
#define CHAN_RESULT     3
#define CHAN_STREAM     4

#define FLV_KEY_FRAME       0x01
#define FLV_INTER_FRAME     0x02
#define APP_NAME "live"

struct RTMP_Message
{
    uint8_t type;
    size_t len;
    unsigned long timestamp;
    uint32_t message_streamID;
    std::string buf;
};
typedef uint8_t Basic_Header;

struct RTMP_Header
{
    Basic_Header flags;
    char timestamp[3];
    char msg_len[3];
    uint8_t msg_type;
    char message_streamID[4];
}PACKED;

struct RTMP_HANDSHAKE
{
    uint8_t flags[8];
    uint8_t random[RANDOM_LEN];
}PACKED;

struct firstPackage
{
    std::string package;
    int status;
    int timestamp;
};

typedef struct RTMP_Header Chunk_Header;
typedef RTMP_Message Message_Header;

#endif /* SRC_APP_NEWDEF_HPP_ */
