/*
 * handshake.hpp
 *
 *  Created on: 2016年3月29日
 *      Author: james
 */
#ifndef SRC_HANDSHAKE_HPP_
#define SRC_HANDSHAKE_HPP_

#include <st.h>
#include<string>
/*
 * this class is base class of handshake protocol
 * return HS_Complete means protocol process ok
 * return HS_Phrase_X means protocol process in X phrase
 */
enum{
    HS_Phrase_0 =  0x10,
    HS_Phrase_1,
    HS_Phrase_2,
    HS_Phrase_3,
    HS_Complete = 0xaa
};
class handshake
{
public:
    handshake(st_netfd_t client_fd);
    virtual ~handshake();

public:
    virtual int process_handshake_protocol(std::string buffer,int len) = 0;
    virtual int get_remain_data_len() = 0;

public:
    int step;

protected:
    int hs_send(const void *buf, int len);

private:
    st_netfd_t st_net_fd;
};

#endif /* SRC_HANDSHAKE_HPP_ */
