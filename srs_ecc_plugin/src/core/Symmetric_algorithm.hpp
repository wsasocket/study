/*
 * Symmetric_algorithm.hpp
 *
 *  Created on: 2016年4月12日
 *      Author: james
 */

#ifndef SRC_CORE_SYMMETRIC_ALGORITHM_HPP_
#define SRC_CORE_SYMMETRIC_ALGORITHM_HPP_
#include <openssl/aes.h>
#include "Ecc_core_define.hpp"
#include <stdint.h>

class Symmetric_algorithm
{
public:
    Symmetric_algorithm();
    virtual ~Symmetric_algorithm();

    int init_symmetric_algorithm(SYMMETRIC_ALG_ID oid);
    int get_sysmmetric_block_size();
    int encrypt(uint8_t *key, uint8_t *iv, uint8_t *in, uint32_t in_length, uint8_t *out);
    int decrypt(uint8_t *key, uint8_t *iv, uint8_t *in, uint32_t in_length, uint8_t *out);

private:
    int aes_encrypt(uint8_t *in, uint8_t *out, size_t length, uint8_t *key, uint8_t *ivec);
    int aes_decrypt(uint8_t *in, uint8_t *out, size_t length, uint8_t *key, uint8_t *ivec);

private:
    int sysmmetric_id;
};

#endif /* SRC_CORE_SYMMETRIC_ALGORITHM_HPP_ */
