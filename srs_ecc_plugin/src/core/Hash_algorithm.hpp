/*
 * hash_algorithm.hpp
 *
 *  Created on: 2016年4月12日
 *      Author: james
 */

#ifndef SRC_CORE_HASH_ALGORITHM_HPP_
#define SRC_CORE_HASH_ALGORITHM_HPP_
#include <openssl/md5.h>
#include <openssl/sha.h>
#include "Ecc_algorithm.hpp"
class Hash_algorithm
{
public:
    Hash_algorithm();
    virtual ~Hash_algorithm();
    int init_hash_algorithm(HASH_ALG_ID oid);
    int hash(uint8_t *in, uint32_t in_length, uint8_t *out);
    int get_hash_out_size();
private:
    int hash_id;
};

#endif /* SRC_CORE_HASH_ALGORITHM_HPP_ */
