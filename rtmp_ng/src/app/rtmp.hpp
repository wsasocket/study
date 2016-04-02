/*
 * rtmp.h
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#ifndef SRC_RTMP_HPP_
#define SRC_RTMP_HPP_

#include "rtmphandshake.hpp"
#include "../core/protocol.hpp"
#include "../3rd/amf.h"
#include"../3rd/utils.h"
#include "rtmpserver.hpp"

/* This class is derived from class protocol
 * we should implement parse_protocol()
 * receive data save in variable "buf"
 * send data should save in variable "send_queue"
 * we can use "st_net_fd" to write data through network
  */

class rtmp: public protocol
{
public:
    rtmp(st_netfd_t client_fd,rtmp_server * srv);
    virtual ~rtmp();

public:
    void parse_protocol();
    handshake * create_handshake(int init_phrase);
    void parse_handshake_protocol();

public:
    rtmp_server * server_ptr;
    handshake * hs;
    bool playing;
    bool ready;
    RTMP_Message messages[64];
    size_t chunk_len;
    uint32_t written_seq;
    uint32_t read_seq;
    amf_object_t metadata;

private:
    firstPackage audioPackage, videoPackage;

private:
    void process_rtmp_protocol();
    void handle_connect(double txid, Decoder *dec);
    void handle_createstream(double txid, Decoder *dec);
    void handle_publish(double txid, Decoder *dec);
    void handle_play(double txid, Decoder *dec);
    void handle_invoke(const RTMP_Message *msg, Decoder *dec);
    void handle_message(RTMP_Message *msg, int header_len);
    void handle_fcpublish(double txid, Decoder *dec);
    void handle_play2(double txid, Decoder *dec);
    void handle_setdataframe(Decoder *dec);
    void handle_AVPackage(int headSize, int payloadSize, int type, int timestamp);
    void cmd_set_peer_BW();
    void cmd_set_chunk_size(int newsize = DEFAULT_CHUNK_LEN);
    void cmd_window_ACK();
    void cmd_connect_reply(double txid);
    void try_to_send();
    void start_playback();
    void rtmp_send(uint8_t type, uint32_t endpoint, const std::string &buf, unsigned long timestamp = 0, int channel_num = CHAN_CONTROL);
    void send_reply(double txid, const AMFValue &reply = AMFValue(), const AMFValue &status = AMFValue());
};

#endif /* SRC_RTMP_HPP_ */
