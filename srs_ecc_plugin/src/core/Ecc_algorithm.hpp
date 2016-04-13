/*
 * Ecc_algorithm.hpp
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#ifndef SRC_CORE_ECC_ALGORITHM_HPP_
#define SRC_CORE_ECC_ALGORITHM_HPP_
#include "Ecc_core_define.hpp"
#include<stdint.h>
#include<stdio.h>
#include <openssl/ec.h>
class Ecc_Algorithm
{
public:
    Ecc_Algorithm();
    virtual ~Ecc_Algorithm();

public:
    int init_ecc_algorithm(CURVE_GROUP_ID oid, size_t hash_len);
    int get_ecc_size();
    int ecc_sign(uint8_t *hash, uint8_t *private_key, uint8_t *sign_r, uint8_t *sign_s);
    int ecc_verify(uint8_t *origin_hash, uint8_t *public_key_x, uint8_t *public_key_y, uint8_t *sign_r, uint8_t *sign_s);
    int ecc_dh_exchange_key(uint8_t *Pub_r_x, uint8_t *Pub_r_y, //peer random number K-point
            uint8_t *Pub_x, uint8_t *Pub_y,  //peer public key
            uint8_t *self_r, //self random number
            uint8_t *self_Pri, // self private key
            uint8_t *shared_key  // result ,for ECC 256,out size is 256 bit
            );
    // for ECC 256,returned result length should increase 64 bytes for package
    int ecc_encrypt(uint8_t *in, size_t in_length, uint8_t *Private_key, uint8_t *out);
    // see above and think
    int ecc_decrypt(uint8_t *in, size_t in_length, uint8_t *Private_key, uint8_t *out);

private:
    int ecc_id;
    size_t hash_len;
    EC_GROUP *group;
};

#endif /* SRC_CORE_ECC_ALGORITHM_HPP_ */
