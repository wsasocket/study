#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <math.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../../objs/include/srs_librtmp.h"
typedef struct live_flv
{
    int fd;
    uint16_t port;
    int finish; //判断eof
    int chunk_left; //http的一个chunk所剩数据
    int total_send; //http一共发送的字节数
    int total_recv; //http一共接收的字节数
    char ip[32];
    char path[64];
    FILE *fp;
} LiveFlv;
int _parse_url(const char *url, char * ip, char * path, uint16_t * port)
{
    char protocol[8] = { 'h', 't', 't', 'p', ':', '/', '/', '\0' };
    char host_name[128];
    char ip_addr[32];
    char str_port[6];
    char app[10];
    char stream[256]; //must be end with '.flv'
    char *p;

    int len = 0;
    int i = 0, j = 0;
    //protocol://host_name:[str_port]/app/stream
    *port = 8080;
    p = (char *) url;
    len = strlen(url);

    if(memcmp(p, protocol, 7) != 0){
        srs_human_trace("url protocol  invalid");
        return -1;
    }
    p += 7;
    i = 0;
    len -= 7;
    bzero(host_name, 128);
    bzero(str_port, 6);
    bzero(app, 10);
    bzero(stream, 256);
    while (p[i] != '/' && i < len){
        if(p[i] == ':'){
            host_name[i] = 0;
            break;
        }
        host_name[i] = p[i];
        i++;
    }
    j = 0;
    if(p[i] == ':'){
        i++;
        while (p[i] != '/' && i < len){
            if(p[i] > '9' || p[i] < '0'){
                srs_human_trace("user defined port invalid");
                return -1;
            }
            str_port[j] = p[i];
            j++;
            i++;
        }
        *port = atoi(str_port);
    }
    j = 0;
    if(p[i] == '/'){
        i++;
        while (p[i] != '/' && i < len){
            app[j] = p[i];
            i++;
            j++;
        }
    }
    if(p[i] != '/'){
        srs_human_trace("CAN NOT find correct APP or stream");
        return -1;
    }
    i++;
    j = 0;
    while (i < len){
        stream[j] = p[i];
        i++;
        j++;
    }

    struct hostent *hptr;
    char **pptr;
    if((hptr = gethostbyname(host_name)) == NULL){
        srs_human_trace("gethostbyname error for host:%s\n", host_name);
        return -1;
    }
    pptr=hptr->h_addr_list;
    sprintf(ip,"%s",inet_ntop(hptr->h_addrtype, *pptr, ip_addr, sizeof(ip_addr)));
    sprintf(path, "/%s/%s", app, stream);
    return 0;
}

//live-flv function
extern LiveFlv *live_flv_create(char *url);
extern int live_flv_destroy(LiveFlv *flv);
extern int live_flv_connect_stream(LiveFlv *flv);
extern int live_flv_play_stream(LiveFlv *flv, int duration, int timeout);
extern int live_flv_is_eof(LiveFlv *flv);

//flv structure function
extern int live_flv_read_header(LiveFlv *flv, char *header);
extern int live_flv_read_tag_header(LiveFlv *flv, char *type, int32_t *size, u_int32_t *timestamp);
extern int live_flv_read_tag_data(LiveFlv *flv, char *data, int size);
extern int live_flv_read_pre_tag_size(LiveFlv *flv);

//http read write function
extern int http_read_chunk_size(LiveFlv *flv);
extern int http_read_nbytes(LiveFlv *flv, char *buf, int len);
extern int http_connect(LiveFlv *flv);
extern int recv_http_res_head(LiveFlv *flv);
extern int send_http_req(LiveFlv *flv);

//public function
extern int parse_url(LiveFlv *flv, const char *url);
extern int sock_init(LiveFlv *flv);
extern int buffer_init(LiveFlv *flv);
extern int str2int(void *str, int start, int len);

int main(int argc, char *argv[])
{
    //user options
    char *url;
    int32_t duration;
    int32_t timeout;

    //live flv
    LiveFlv *flv;
    int ret;

    //time
    int64_t time_startup = 0;
    int64_t time_connect_stream = 0;
    int64_t time_cleanup = 0;
    int32_t time_duration = 0;

    //bytes
    int64_t bytes_nrecv = 0;

    printf("detect flv stream\n");
    if(argc <= 3){
        printf("detect stream on server, print result to stderr.\n"
                "Usage: %s <url> <duration> <timeout>\n"
                "   url         flv stream url to play\n"
                "   duration    how long to play, in seconds, stream time.\n"
                "   timeout     how long to timeout, in seconds, system time.\n"
                "For example:\n"
                "   %s http://127.0.0.1:8080/live/livestream 3 10\n", argv[0], argv[0]);
        exit(-1);
    }

    url = argv[1];
    duration = atoi(argv[2]);
    timeout = atoi(argv[3]);

    srs_human_trace("flv url: %s", url);
    srs_human_trace("duration: %ds, timeout:%ds", duration, timeout);

    if(duration <= 0 || timeout <= 0){
        srs_human_trace("duration and timeout must be positive.");
        exit(-2);
    }

    time_startup = srs_utils_time_ms();
    if((flv = live_flv_create(url)) == NULL){
        srs_human_trace("live flv stream create failed");
        exit(-3);
    }

    if((ret = live_flv_connect_stream(flv)) != 0){
        srs_human_trace("connect server failed");
        goto LIVE_FLV_EXIT;
    }
    srs_human_trace("connect live flv server success");
    time_connect_stream = srs_utils_time_ms();

    timeout = timeout * 1000 - (time_connect_stream - time_startup);
    duration *= 1000;
    if((ret = live_flv_play_stream(flv, duration, timeout)) != 0){
        srs_human_trace("parse live flv failed");
        goto LIVE_FLV_EXIT;
    }

    LIVE_FLV_EXIT: bytes_nrecv = flv->total_recv;
    live_flv_destroy(flv);
    time_cleanup = srs_utils_time_ms();
    time_duration = (int) (time_cleanup - time_startup);

    fprintf(stderr, "{"
            "\"%s\":%d, " //#0
            "\"%s\":%d, "//#1
            "\"%s\":%d, "//#2
            "\"%s\":%d, "//#3
            "\"%s\":%d  "//#4
            "}", "code", ret, //#0
            "total", time_duration, //#1
            "connect", (int) (time_connect_stream - time_startup), //#2
            "play", (int) (time_cleanup - time_connect_stream), //#3
            "play_kbps", (int) ((time_duration <= 0) ? 0 : (bytes_nrecv * 8 / time_duration)) //#4
            );

    return 0;
}

/*
 *   创建一个live-flv对象
 *   @param1 url: flv流地址
 *   @return: 成功返回对象,失败返回NULL
 */
LiveFlv *live_flv_create(char *url)
{
    LiveFlv *flv;

    if((flv = (LiveFlv *) malloc(sizeof(LiveFlv))) == NULL){
        srs_human_trace("malloc failed");
        return NULL;
    }

    flv->finish = 0;
    flv->chunk_left = 0;

    if(parse_url(flv, url) != 0){
        srs_human_trace("parse url failed");
        return NULL;
    }

    return flv;
}

/*
 *   删除一个live-flv对象
 */
int live_flv_destroy(LiveFlv *flv)
{
    if(flv != NULL){
        free(flv);
    }

    return 0;
}

/*
 *   连接到live-flv服务器
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_connect_stream(LiveFlv *flv)
{
    if(sock_init(flv) != 0){
        srs_human_trace("init socket failed");
        return -1;
    }

    if(buffer_init(flv) != 0){
        srs_human_trace("create buffer failed");
        return -1;
    }

    if(http_connect(flv) != 0){
        srs_human_trace("live flv connect failed");
        return -1;
    }

    return 0;
}

/*
 *   播放live-flv流
 *   @param1 flv: 流对象
 *   @param2 duration: 播放时间
 *   @param3 timeout: 超时时间
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_play_stream(LiveFlv *flv, int duration, int timeout)
{
    int ret;
    int size;
    char type;
    char header[13];
    char *data = NULL;

    u_int32_t timestamp = 0;
    u_int32_t basetime = 0;
    int64_t time_start = srs_utils_time_ms();

    //flv header
    if(live_flv_read_header(flv, header) != 0){
        srs_human_trace("read flv header failed");
        return -1;
    }

    while (1){
        //tag header
        if(live_flv_read_tag_header(flv, &type, &size, &timestamp) != 0){
            if(live_flv_is_eof(flv)){
                srs_human_trace("parse completed.");
                return 0;
            }
            srs_human_trace("flv get packet failed.");
            return -1;
        }

        if(basetime <= 0){
            basetime = timestamp;
        }

        if(size <= 0){
            srs_human_trace("invalid size=%d", size);
            return -1;
        }

        //tag data
        data = (char*) malloc(size);
        if(live_flv_read_tag_data(flv, data, size) != 0){
            srs_human_trace("read flv tag data failed");
            return -1;
        }

        if(srs_human_print_rtmp_packet(type, timestamp, data, size) != 0){
            srs_human_trace("parse packet failed");
            return -1;
        }
        free(data);

        //pre tag size
        if(live_flv_read_pre_tag_size(flv) != 0){
            srs_human_trace("http read tagsize failed");
            return -1;
        }

        if(srs_utils_time_ms() - time_start > timeout){
            srs_human_trace("timeout, terminate");
            return 0;
        }

        if(timestamp - basetime > duration){
            srs_human_trace("duration exceed, terminate");
            return 0;
        }
    }
}

/*
 *   读取flv头部
 *   @param1 flv: 流对象
 *   @param2 header: 用来存储头部信息的地址
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_read_header(LiveFlv *flv, char *header)
{
    if(http_read_nbytes(flv, header, 13) != 0){
        srs_human_trace("get flv header failed");
        return -1;
    }

    return 0;
}

/*
 *   读取flv的tag头部
 *   @param1 flv: 流对象
 *   @param2 type: 存储tag类型
 *   @param3 size: 存储tag大小
 *   @param4 timestamp: 存储tag时间戳
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_read_tag_header(LiveFlv *flv, char *type, int32_t *size, u_int32_t *timestamp)
{
    char buf[11];

    if(http_read_nbytes(flv, buf, 11) != 0){
        return -1;
    }

    *type = buf[0];
    *size = str2int(buf, 1, 3);
    *timestamp = str2int(buf, 4, 3);

    return 0;
}

/*
 *   读取flv的tag数据
 *   @param1 flv: 流对象
 *   @param2 data: 存储tag数据的地址
 *   @param3 size: 读取数据的长度
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_read_tag_data(LiveFlv *flv, char *data, int size)
{
    if(http_read_nbytes(flv, data, size) != 0){
        srs_human_trace("read tag data failed");
        return -1;
    }

    return 0;
}

/*
 *   读取flv的pre_tag_size数据
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_read_pre_tag_size(LiveFlv *flv)
{
    char buf[4];

    if(http_read_nbytes(flv, buf, 4) != 0){
        srs_human_trace("read tag size failed");
        return -1;
    }

    return 0;
}

/*
 *   读取http的chunk大小
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int http_read_chunk_size(LiveFlv *flv)
{
    int i = 0;
    char chunk_head[32];

    if(fgets(chunk_head, 32, flv->fp) == NULL){
        if(feof(flv->fp)){
            flv->finish = 1;
        }
        srs_human_trace("http read chunk size failed");
        return -1;
    }

    int j = 0;
    int chunk_size = 0;
    while (chunk_head[j] != '\r'){
        if(chunk_head[j] >= '0' && chunk_head[j] <= '9'){
            chunk_size = chunk_size * 16 + chunk_head[j] - 48;
        } else if(chunk_head[j] >= 'a' && chunk_head[j] <= 'f'){
            chunk_size = chunk_size * 16 + chunk_head[j] - 87;
        } else if(chunk_head[j] >= 'A' && chunk_head[j] <= 'F'){
            chunk_size = chunk_size * 16 + chunk_head[j] - 55;
        } else{
            srs_human_trace("http read chunk size got invalid charactor");
            return -1;
        }
        j++;
    }

    flv->total_recv += strlen(chunk_head);
    return chunk_size;
}

/*
 *   读取http的n字节数据
 *   @param1 flv: 流对象
 *   @param2 buf: 数据存储的地址
 *   @param3 len: 读取数据的长度
 *   @return: 成功返回0, 失败返回-1
 */
int http_read_nbytes(LiveFlv *flv, char *buf, int len)
{
    int nread = 0;
    int nleft = len;
    int toread;

    while (nleft > 0){
        if(flv->chunk_left == 0){
            if((flv->chunk_left = http_read_chunk_size(flv)) == -1){
                srs_human_trace("http read chunk size failed");
                return -1;
            }
        }

        toread = nleft > flv->chunk_left ? flv->chunk_left : nleft;
        if((fread(buf + nread, 1, toread, flv->fp)) != toread){
            srs_human_trace("fread failed");
            return -1;
        } else{
            flv->chunk_left -= toread;
            nread += toread;
            nleft -= toread;
            flv->total_recv += toread;
        }
    }

    return 0;
}

/*
 *   解析url
 *   @param1 flv: 流对象
 *   @param2 url: url
 *   @return: 成功返回0, 失败返回-1
 */
int parse_url(LiveFlv *flv, const char *url)
{
    _parse_url(url, flv->ip, flv->path, &flv->port);
    srs_human_trace("ip, port, path:%s, %d, %s", flv->ip, flv->port, flv->path);
    return 0;
}

/*
 *   初始化socet套接字并连接到server
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int sock_init(LiveFlv *flv)
{
    if((flv->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        srs_human_trace("socket failed:%s", strerror(errno));
        return -1;
    }

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(flv->port);
    client.sin_addr.s_addr = inet_addr(flv->ip);

    if(connect(flv->fd, (struct sockaddr*) &client, sizeof(struct sockaddr)) != 0){
        srs_human_trace("socket connect failed");
        return -1;
    }

    return 0;
}

/*
 *   初始化缓冲区
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int buffer_init(LiveFlv *flv)
{
    if((flv->fp = fdopen(flv->fd, "rw+")) == NULL){
        srs_human_trace("fdopen failed");
        return -1;
    }

    return 0;
}

/*
 *   建立http连接
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int http_connect(LiveFlv *flv)
{
    if(send_http_req(flv) != 0){
        srs_human_trace("send http req failed");
        return -1;
    }

    if(recv_http_res_head(flv) != 0){
        srs_human_trace("recv http res head failed");
        return -1;
    }
}

/*
 *   获取http的响应头
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int recv_http_res_head(LiveFlv *flv)
{
    char buf[256];

    memset(buf, 0, sizeof(buf));
    while ((buf[0] != '\r') || (buf[1] != '\n')){
        if(fgets(buf, 255, flv->fp) == NULL){
            srs_human_trace("fgets error");
            return -1;
        }
        flv->total_recv += strlen(buf);
        srs_human_trace("%s", buf);
    }

    return 0;
}

/*
 *   发送http的请求
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int send_http_req(LiveFlv *flv)
{
    char req[256];
    int len;

    len = sprintf(req, "GET %s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", flv->path, flv->ip, flv->port);
    srs_human_trace("%s", req);
    if(fwrite(req, 1, len, flv->fp) != len){
        srs_human_trace("socket send error");
        return -1;
    }
    flv->total_send += len;

    return 0;
}

/*
 *   判断流是否结束
 *   @param1 flv: 流对象
 *   @return: 成功返回0, 失败返回-1
 */
int live_flv_is_eof(LiveFlv *flv)
{
    return flv->finish;
}

/*
 *   以整数形式读取指定字节数据
 *   @param1 flv: 流对象
 *   @param2 start: 指定字节的起始位置
 *   @param3 len: 读取长度
 *   @return: 成功返回0, 失败返回-1
 */
int str2int(void *str, int start, int len)
{
    unsigned char *p = (unsigned char*) str + start;
    int sum = 0;
    int i = len;

    while (i > 0){
        sum += *p++ * pow(256, --i);
    }

    return sum;
}
