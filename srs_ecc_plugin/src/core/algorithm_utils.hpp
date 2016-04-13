/*
 * algorithm_utils.hpp
 *
 *  Created on: 2016年4月12日
 *      Author: james
 */

#ifndef SRC_CORE_ALGORITHM_UTILS_HPP_
#define SRC_CORE_ALGORITHM_UTILS_HPP_
#include <stdint.h>
#include<stdio.h>
class algorithm_utils
{
public:
    algorithm_utils();
    virtual ~algorithm_utils();

    static char *base64_encoder(const uint8_t * input, int length, bool with_new_line = true);
    static int get_base64_decode_len(char *base64);
    static uint8_t *base64_decoder(char * input, int length, bool with_new_line = true);
    static uint8_t *kdf_with_md5(const uint8_t *msg, size_t msg_len, size_t key_len);
    static uint8_t *kdf_with_sha256(const uint8_t *msg, size_t msg_len, size_t key_len);
};

#endif /* SRC_CORE_ALGORITHM_UTILS_HPP_ */
