/*
 * Hash_algorithm.cpp
 *
 *  Created on: 2016年4月12日
 *      Author: james
 */

#include "Hash_algorithm.hpp"

Hash_algorithm::Hash_algorithm()
{
    // TODO Auto-generated constructor stub
    hash_id = -1;
}

Hash_algorithm::~Hash_algorithm()
{
    // TODO Auto-generated destructor stub
}

int Hash_algorithm::init_hash_algorithm(HASH_ALG_ID oid)
{
    hash_id = oid;
    return get_hash_out_size();
}

int Hash_algorithm::get_hash_out_size()
{
    if(hash_id == -1)
        return -1;

    if(hash_id == MD_5 )
        return MD5_DIGEST_LENGTH;

    if(hash_id == SHA_1)
        return SHA_DIGEST_LENGTH;

    return SHA256_DIGEST_LENGTH;
}

int Hash_algorithm::hash(uint8_t *in, uint32_t in_length, uint8_t *out)
{
    if(hash_id == -1)
        return ALGORITHM_NOT_INITILIZE;

    typedef uint8_t *(*HASH)(const uint8_t *, size_t, uint8_t *);
    HASH hash_func = NULL;

    switch (hash_id)
    {
        case SM_3:
            //TODO add SM3
            break;
        case SHA_1:
            hash_func = SHA1;
            break;
        case SHA_256:
            hash_func = SHA256;
            break;
        case MD_5:
            hash_func = MD5;
            break;
        default:
            return ALGORITHM_NOT_DEFINE;
    }
    if(hash_func == NULL)
        return ALGORITHM_NOT_DEFINE;

    if(hash_func(in, in_length, out) == NULL)
        return ALGORITHM_CALC_ERROR;

    return get_hash_out_size();
}

