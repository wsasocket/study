/*
 * rtmp.cpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <vector>
#include "rtmp.hpp"

rtmp::rtmp(st_netfd_t client_fd, rtmp_server * srv) :
        protocol(client_fd)
{
    server_ptr = srv;
    handshake_ptr = create_handshake(HS_Phrase_0);
    written_seq = 0;
    read_seq = 0;
    ready = false;
    playing = false;

    for (int i = 0; i < 64; ++i){
        messages[i].timestamp = 0;
        messages[i].len = 0;
    }
    chunk_len = DEFAULT_CHUNK_LEN;
    audioPackage.status = 0;
    videoPackage.status = 0;
}

rtmp::~rtmp()
{
}

void rtmp::close_connect()
{
    st_netfd_close(st_net_fd);
    st_netfd_free(st_net_fd);

    if(server_ptr->publisher == this){
        _trace("%s", "Publisher disconnect,Notify all player");
        std::vector<rtmp *>::iterator i;
        rtmp *player;
        for (i = server_ptr->client_list.begin(); i != server_ptr->client_list.end(); i++){
            player = *i;
            player->ready = false;
        }
        server_ptr->publisher = NULL;
    }
    std::vector<rtmp *>::iterator i;
    rtmp *player;
    for (i = server_ptr->client_list.begin(); i != server_ptr->client_list.end(); i++){
        player = *i;
        if(player == this){
            player->buf.clear();
            player->send_queue.clear();
            delete handshake_ptr;
            handshake_ptr = NULL;
            server_ptr->client_list.erase(i);
            break;
        }
    }
}
void rtmp::parse_handshake_protocol()
{
    handshake_ptr->do_handshake();
}

handshake * rtmp::create_handshake(int init_phrase)
{
    return new rtmp_handshake(st_net_fd, init_phrase);
}

void rtmp::parse_protocol()
{
    std::string recv_buffer(MAX_BUF_LEN, 0);
    struct in_addr *from = (struct in_addr *) st_netfd_getspecific(st_net_fd);
    ssize_t read_size;
    while (true){
        read_size = st_read(st_net_fd, &recv_buffer[0], MAX_BUF_LEN, SEC2USEC(REQUEST_TIMEOUT));
        if(read_size == -1 && errno == ETIME){
            continue;
        }
        if(read_size == 0){
            _error("Network connection from %s is closed.", inet_ntoa(*from));
            return;
        }
        buf.append(recv_buffer, 0, read_size);
        // begin parse
        while (!buf.empty()){
            uint8_t flags = buf[0];
            static const size_t HEADER_LENGTH[] = { 12, 8, 4, 1 };
            size_t header_len = HEADER_LENGTH[flags >> 6];
            if(buf.size() < header_len)
                break;

            Chunk_Header header;
            memcpy(&header, buf.data(), header_len);
            Message_Header *msg = &messages[flags & 0x3f];
            if(header_len >= 8){
                msg->len = load_be24(header.msg_len);
                if(msg->len < msg->buf.size()){
                    _error("RTMP package parse error [%x]", ERROR_RTMP_PARSE);
                    throw std::runtime_error("Invalid message length");
                }
                msg->type = header.msg_type;
            }
            if(header_len >= 12){
                msg->message_streamID = load_le32(header.message_streamID);
            }
            if(msg->len == 0){
                _error("RTMP package parse error [%x]", ERROR_RTMP_PARSE);
                throw std::runtime_error("Message without a header");
            }

            size_t chunkSize = msg->len - msg->buf.size();
            if(chunkSize > chunk_len)
                chunkSize = chunk_len;
            if(buf.size() < header_len + chunkSize){
                break;
            }

            if(header_len >= 4){
                unsigned long ts = load_be24(header.timestamp);
                if(ts == 0xffffff){
                    _error("RTMP package parse error [%x]", ERROR_RTMP_PARSE);
                    throw std::runtime_error("Ext timestamp not supported");
                }
                if(header_len < 12){
                    ts += msg->timestamp;
                }
                msg->timestamp = ts;
            }

            msg->buf.append(buf, header_len, chunkSize);
            if(msg->type == MSG_AUDIO || msg->type == MSG_VIDEO)
                handle_AVPackage(header_len, chunkSize, msg->type, msg->timestamp);

            buf.erase(0, header_len + chunkSize);
            if(msg->buf.size() == msg->len){
                handle_message(msg, header_len);
                msg->buf.clear();
            }
        }
    }
}

void rtmp::try_to_send()
{
    size_t len = send_queue.size();
    if(len > MAX_BUF_LEN)
        len = MAX_BUF_LEN;

    ssize_t written = st_write(st_net_fd, send_queue.data(), len, ST_UTIME_NO_TIMEOUT);
    if(written == -1){
        if(errno == EINTR){
            _error("Interrupt by user [%x]", ERROR_RTMP_ST_NET_WRITE);
            return;
        }
        _error("Unable write to client [%x]", ERROR_RTMP_ST_NET_WRITE);
        throw std::runtime_error("Unable write to client");
    }

    send_queue.erase(0, written);
}

void rtmp::rtmp_send(uint8_t type, uint32_t endpoint, const std::string &buf, unsigned long timestamp, int channel_num)
{
    if(endpoint == STREAM_ID)
        channel_num = CHAN_STREAM;

    Chunk_Header header;
    header.flags = (channel_num & 0x3f) | (0 << 6);
    header.msg_type = type;
    set_be24(header.timestamp, timestamp);
    set_be24(header.msg_len, buf.size());
    set_le32(header.message_streamID, endpoint);

    send_queue.append((char *) &header, sizeof header);
    written_seq += sizeof header;
    size_t pos = 0;
    while (pos < buf.size()){
        if(pos){
            uint8_t flags = (channel_num & 0x3f) | (3 << 6);
            send_queue += char(flags);
            written_seq += 1;
        }
        size_t chunk = buf.size() - pos;
        if(chunk > chunk_len)
            chunk = chunk_len;
        send_queue.append(buf, pos, chunk);
        written_seq += chunk;
        pos += chunk;
    }

    try_to_send();
}

void rtmp::send_reply(double txid, const AMFValue &reply, const AMFValue &status)
{
    if(txid <= 0.0)
        return;
    Encoder invoke;
    amf_write(&invoke, std::string("_result"));
    amf_write(&invoke, txid);
    amf_write(&invoke, reply);
    amf_write(&invoke, status);
    rtmp_send(MSG_INVOKE_AMF0, CONTROL_ID, invoke.buf, 0, CHAN_RESULT);
}

void rtmp::cmd_window_ACK()
{
    uint32_t size;
    std::string buf;
    size = htonl(2500000);
    buf.append((char *) (&size), 4);
    rtmp_send(MSG_WINDOW_SIZE, 0, buf);
}

void rtmp::cmd_set_chunk_size(int newsize)
{
    std::string buf;
    uint32_t chunk_size;
    chunk_size = htonl(newsize);
    buf.append((char *) (&chunk_size), 4);
    rtmp_send(MSG_SET_CHUNK, 0, buf);
    _trace("Set chunk size to %d", newsize);
}

void rtmp::cmd_set_peer_BW()
{
    std::string buf;
    uint32_t size;
    uint8_t limit = 2;
    size = htonl(2500000);
    buf.append((char *) (&size), 4);
    buf.append((char *) (&limit), 1);
    rtmp_send(MSG_SET_PEER_WINDOW_SIZE, 0, buf);
}

void rtmp::cmd_connect_reply(double txid)
{
    amf_object_t version;
    version.insert(std::make_pair("fmsVer", std::string("FMS/4,5,1,484")));
    version.insert(std::make_pair("capabilities", 255.0));
    version.insert(std::make_pair("mode", 1.0));
    amf_object_t status;
    status.insert(std::make_pair("level", std::string("status")));
    status.insert(std::make_pair("code", std::string("NetConnection.Connect.Success")));
    status.insert(std::make_pair("description", std::string("Connection succeeded.")));
    status.insert(std::make_pair("objectEncoding", 3.0));
    _trace("Send Connect reply to [%x]", st_net_fd);
    send_reply(txid, version, status);
}

void rtmp::handle_connect(double txid, Decoder *dec)
{
    amf_object_t params = amf_load_object(dec);
    std::string app = get(params, std::string("app")).as_string();
    std::string ver = "(unknown)";
    AMFValue flashver = get(params, std::string("flashVer"));
    if(flashver.type() == AMF_STRING)
        ver = flashver.as_string();

    if(app != APP_NAME){
        _error("Application unsupport[%x]", ERROR_RTMP_UNSUPPORT_APP);
        throw std::runtime_error("Unsupported application: " + app);
    }
    _trace("Client  %s (version %s) connected  :", app.c_str(), ver.c_str());
    cmd_window_ACK();
    cmd_set_peer_BW();
    cmd_set_chunk_size(chunk_len);
    cmd_connect_reply(txid);
}

void rtmp::handle_createstream(double txid, Decoder *dec)
{
    send_reply(txid, AMFValue(), double(STREAM_ID));
}

void rtmp::handle_publish(double txid, Decoder *dec)
{
    amf_load(dec);
    std::string path = amf_load_string(dec);
    _trace("publish %s", path.c_str());
    amf_object_t status;
    status.insert(std::make_pair("level", std::string("status")));
    status.insert(std::make_pair("code", std::string("NetStream.Publish.Start")));
    status.insert(std::make_pair("description", std::string("Stream is now published.")));
    status.insert(std::make_pair("details", path));
    Encoder invoke;
    amf_write(&invoke, std::string("onStatus"));
    amf_write(&invoke, 0.0);
    amf_write_null(&invoke);
    amf_write(&invoke, status);
    rtmp_send(MSG_INVOKE_AMF0, STREAM_ID, invoke.buf);
    send_reply(txid);
}

void rtmp::handle_play(double txid, Decoder *dec)
{
    amf_load(dec);
    std::string path = amf_load_string(dec);
    _trace("play %s", path.c_str());
    start_playback();
    send_reply(txid);
}

void rtmp::handle_fcpublish(double txid, Decoder *dec)
{
    if(server_ptr->publisher != NULL)
        throw std::runtime_error("Already have a publisher");
    server_ptr->publisher = this;
    _trace("info %s ", "publisher connected.");

    std::vector<rtmp *>::iterator i;
    for (i = server_ptr->client_list.begin(); i != server_ptr->client_list.end(); i++){
        rtmp * receiver = *i;
        if(receiver != server_ptr->publisher && receiver != NULL){
            receiver->cmd_set_chunk_size(server_ptr->publisher->chunk_len);
            receiver->chunk_len = server_ptr->publisher->chunk_len;
            _trace("Pub->Client chunk size to %d ", receiver->chunk_len);
        }
    }

    amf_load(dec);
    std::string path = amf_load_string(dec);
    _trace("fcpublish %s", path.c_str());
    amf_object_t status;
    status.insert(std::make_pair("code", std::string("NetStream.Publish.Start")));
    status.insert(std::make_pair("description", path));
    Encoder invoke;
    amf_write(&invoke, std::string("onFCPublish"));
    amf_write(&invoke, 0.0);
    amf_write_null(&invoke);
    amf_write(&invoke, status);
    rtmp_send(MSG_INVOKE_AMF0, CONTROL_ID, invoke.buf);
    send_reply(txid);
}

void rtmp::start_playback()
{
    amf_object_t status;
    status.insert(std::make_pair("level", std::string("status")));
    status.insert(std::make_pair("code", std::string("NetStream.Play.Reset")));
    status.insert(std::make_pair("description", std::string("Resetting and playing stream.")));
    Encoder invoke;
    amf_write(&invoke, std::string("onStatus"));
    amf_write(&invoke, 0.0);
    amf_write_null(&invoke);
    amf_write(&invoke, status);
    rtmp_send(MSG_INVOKE_AMF0, STREAM_ID, invoke.buf);
    status.clear();
    status.insert(std::make_pair("level", std::string("status")));
    status.insert(std::make_pair("code", std::string("NetStream.Play.Start")));
    status.insert(std::make_pair("description", std::string("Started playing.")));
    invoke.buf.clear();
    amf_write(&invoke, std::string("onStatus"));
    amf_write(&invoke, 0.0);
    amf_write_null(&invoke);
    amf_write(&invoke, status);
    rtmp_send(MSG_INVOKE_AMF0, STREAM_ID, invoke.buf);
    invoke.buf.clear();
    amf_write(&invoke, std::string("|RtmpSampleAccess"));
    amf_write(&invoke, true);
    amf_write(&invoke, true);
    rtmp_send(MSG_NOTIFY_AMF0, STREAM_ID, invoke.buf);
    playing = true;
    ready = false;

    if(server_ptr->publisher != NULL){
        Encoder notify;
        amf_write(&notify, std::string("onMetaData"));
        amf_write_ecma(&notify, metadata);
        rtmp_send(MSG_NOTIFY_AMF0, STREAM_ID, notify.buf);
    }
}

void rtmp::handle_play2(double txid, Decoder *dec)
{
    amf_load(dec);
    amf_object_t params = amf_load_object(dec);
    std::string path = get(params, std::string("streamName")).as_string();
    _trace("play2 %s\n", path.c_str());
    start_playback();
    send_reply(txid);
}

void rtmp::handle_setdataframe(Decoder *dec)
{
    std::string type = amf_load_string(dec);
    if(type != "onMetaData"){
        throw std::runtime_error("Can only set metadata");
    }
    metadata = amf_load_ecma(dec);
    Encoder notify;
    amf_write(&notify, std::string("onMetaData"));
    amf_write_ecma(&notify, metadata);

    std::vector<rtmp *>::iterator i;
    for (i = server_ptr->client_list.begin(); i != server_ptr->client_list.end(); i++){
        rtmp * receiver = *i;
        if(receiver != NULL && receiver->playing){
            receiver->rtmp_send( MSG_NOTIFY_AMF0, STREAM_ID, notify.buf);
        }
    }
}

void rtmp::handle_AVPackage(int headSize, int payloadSize, int type, int timestamp)
{
    if(MSG_AUDIO == type){
        if(timestamp == 0 && audioPackage.status == 0){
            audioPackage.package.append(buf.data(), headSize + payloadSize);
            return;
        }
        if(timestamp == 0 && audioPackage.status == 2){
            audioPackage.package.clear();
            audioPackage.package.append(buf.data(), headSize + payloadSize);
            _trace("AVPackage:%s", "First audio package refreshed!");
            audioPackage.status = 0;
            return;
        }
        if(timestamp != 0){
            if(audioPackage.status != 2)
                audioPackage.status = 2;
        }
        std::vector<rtmp *>::iterator i;
        for (i = server_ptr->client_list.begin(); i != server_ptr->client_list.end(); i++){
            rtmp *receiver = *i;
            if(receiver == this)
                continue;
            if(receiver != NULL && receiver->ready){
                receiver->send_queue.append(buf.data(), headSize + payloadSize);
                receiver->written_seq += (headSize + payloadSize);
                receiver->try_to_send();
            }
        }
    }
    if(MSG_VIDEO == type){
        if(timestamp == 0 && videoPackage.status == 0)
            videoPackage.package.append(buf.data(), headSize + payloadSize);
        if(timestamp == 0 && videoPackage.status == 2){
            videoPackage.package.clear();
            videoPackage.package.append(buf.data(), headSize + payloadSize);
            _trace("AVPackage:%s", "First video package refreshed!");
            videoPackage.status = 0;
        }
        if(timestamp != 0){
            if(videoPackage.status != 2)
                videoPackage.status = 2;
        }

        uint8_t flags = buf[headSize];
        std::vector<rtmp *>::iterator i;
        for (i = server_ptr->client_list.begin(); i != server_ptr->client_list.end(); i++){
            rtmp *receiver = *i;
            if(receiver == this)
                continue;
            if(receiver != NULL){
                if(flags >> 4 == FLV_KEY_FRAME && !receiver->ready){
                    receiver->cmd_set_chunk_size(this->chunk_len);
                    std::string control;
                    uint16_t type = htons(CONTROL_STREAM_BEGIN);
                    control.append((char *) &type, 2);
                    uint32_t stream = htonl(STREAM_ID);
                    control.append((char *) &stream, 4);
                    receiver->rtmp_send(MSG_USER_CONTROL, CONTROL_ID, control);
                    receiver->ready = true;
                    if(timestamp != 0){
                        receiver->send_queue.append(videoPackage.package.data(), videoPackage.package.size());
                        receiver->send_queue.append(audioPackage.package.data(), audioPackage.package.size());
                        receiver->written_seq += videoPackage.package.size();
                        receiver->written_seq += audioPackage.package.size();
                        _trace("Time stamp[%d] begin send to client", timestamp);
                        receiver->try_to_send();
                    }
                }
                if(receiver->ready){
                    receiver->send_queue.append(buf.data(), headSize + payloadSize);
                    receiver->written_seq += (headSize + payloadSize);
                    receiver->try_to_send();
                }
            }
        }
    }
}

void rtmp::handle_invoke(const RTMP_Message *msg, Decoder *dec)
{
    std::string method = amf_load_string(dec);
    double txid = amf_load_number(dec);
    if(msg->message_streamID == CONTROL_ID){
        if(method == "connect"){
            handle_connect(txid, dec);
            return;
        }
        if(method == "FCPublish"){
            handle_fcpublish(txid, dec);
            return;
        }
        if(method == "createStream"){
            handle_createstream(txid, dec);
            return;
        }
        if(method == "releaseStream"){
            _trace("Find releaseStream command");
            return;
        }
        if(method == "_checkbw"){
            handle_createstream(txid, dec);
            _trace("Find _checkbw command");
            return;
        }
        _trace("Invoked message [%s] streamID [%d] from [%X]NOT handled", method.c_str(), msg->message_streamID, st_net_fd);
        return;
    }

    if(msg->message_streamID == STREAM_ID){
        if(method == "publish"){
            handle_publish(txid, dec);
            return;
        }
        if(method == "play"){
            handle_play(txid, dec);
            return;
        }
        if(method == "play2"){
            handle_play2(txid, dec);
            return;
        }
        _trace("Invoked message [%s] at streamID [%d] but server can not handled", method.c_str(), msg->message_streamID);
        return;
    }
}

void rtmp::handle_message(RTMP_Message *msg, int header_len)
{
    size_t pos = 0;
    switch (msg->type)
    {
        case MSG_SET_CHUNK:
        {
            if(pos + 4 > msg->buf.size()){
                throw std::runtime_error("Not enough data");
            }
            chunk_len = load_be32(&msg->buf[pos]);
            _trace("Current connect set chunk size to %d", chunk_len);
            break;
        }
        case MSG_ACK:
        {
            if(pos + 4 > msg->buf.size()){
                throw std::runtime_error("Not enough data");
            }
            read_seq = load_be32(&msg->buf[pos]);
            break;
        }
        case MSG_USER_CONTROL:
        {
            short eventType;
            eventType = load_be16(msg->buf.data());
            if(eventType == CONTROL_BUFFER_TIME){
                int streamID, bufferSize;
                streamID = load_be32(&msg->buf[2]);
                bufferSize = load_be32(&msg->buf[6]);
                _trace("Client set stream %d buffer size %d ms", streamID, bufferSize);
            } else{
                _trace("client request user control eventType is %d", eventType);
                hexdump(msg->buf.data(), msg->buf.size());
            }
            break;
        }
        case MSG_WINDOW_SIZE:
        {
            int windSize;
            windSize = load_be32(&msg->buf[0]);
            _trace("Client acknowledge windows size %d", windSize);
            break;
        }
        case MSG_INVOKE_AMF0:
        {
            Decoder dec;
            dec.version = 0;
            dec.buf = msg->buf;
            dec.pos = 0;
            handle_invoke(msg, &dec);
            break;
        }
        case MSG_INVOKE_AMF3:
        {
            Decoder dec;
            dec.version = 0;
            dec.buf = msg->buf;
            dec.pos = 1;
            handle_invoke(msg, &dec);
        }
            break;
        case MSG_NOTIFY_AMF0:
        {
            Decoder dec;
            dec.version = 0;
            dec.buf = msg->buf;
            dec.pos = 0;
            std::string type = amf_load_string(&dec);
            _trace("notify %s", type.c_str());
            if(msg->message_streamID == STREAM_ID){
                if(type == "@setDataFrame"){
                    handle_setdataframe(&dec);
                }
            }
            break;
        }
        case MSG_AUDIO:
            break;
        case MSG_VIDEO:
            break;
        case MSG_AGGREGATE_MSG:
            throw std::runtime_error("streaming FLV not supported");
            break;
        default:
            _trace("Unhandled message: %02x", msg->type)
            ;
            hexdump(msg->buf.data(), msg->buf.size());
            break;
    }
}

