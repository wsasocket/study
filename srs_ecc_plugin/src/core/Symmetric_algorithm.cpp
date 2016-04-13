/*
 * Symmetric_algorithm.cpp
 *
 *  Created on: 2016年4月12日
 *      Author: james
 */

#include "Symmetric_algorithm.hpp"

Symmetric_algorithm::Symmetric_algorithm()
{
    // TODO Auto-generated constructor stub
    sysmmetric_id = -1;
}

Symmetric_algorithm::~Symmetric_algorithm()
{
    // TODO Auto-generated destructor stub
}

int Symmetric_algorithm::init_symmetric_algorithm(SYMMETRIC_ALG_ID oid)
{
    sysmmetric_id = oid;
    return get_sysmmetric_block_size();
}

int Symmetric_algorithm::get_sysmmetric_block_size()
{
    //TODO aes block size if change modify return value
    return sysmmetric_id == -1 ? -1 : 16;
}

int Symmetric_algorithm::encrypt(uint8_t *key, uint8_t *iv, uint8_t *in, uint32_t in_length, uint8_t *out)
{
    if(sysmmetric_id == -1)
        return ALGORITHM_NOT_INITILIZE;
    switch (sysmmetric_id)
    {
        case SM_4_CBC:
            //TODO add sm4
            break;
        case AES_128_CBC:
            aes_encrypt(in, out, in_length, key, iv);
            break;
        default:
            return ALGORITHM_NOT_DEFINE;
            break;
    }
    return get_sysmmetric_block_size();
}

int Symmetric_algorithm::decrypt(uint8_t *key, uint8_t *iv, uint8_t *in, uint32_t in_length, uint8_t *out)
{
    if(sysmmetric_id == -1)
        return ALGORITHM_NOT_INITILIZE;
    switch (sysmmetric_id)
    {
        case SM_4_CBC:
            //TODO add sm4
            break;
        case AES_128_CBC:
            aes_decrypt(in, out, in_length, key, iv);
            break;
        default:
            return ALGORITHM_NOT_DEFINE;
            break;
    }
    return get_sysmmetric_block_size();
}

int Symmetric_algorithm::aes_encrypt(uint8_t *in, uint8_t *out, size_t length, uint8_t *key, uint8_t *ivec)
{
    AES_KEY encrypt_key;
    if(AES_set_encrypt_key(key, 128, &encrypt_key) != 0)
        return ALGORITHM_KEY_ERROR;
    AES_cbc_encrypt(in, out, length, &encrypt_key, ivec, AES_ENCRYPT);
    return length;
}

int Symmetric_algorithm::aes_decrypt(uint8_t *in, uint8_t *out, size_t length, uint8_t *key, uint8_t *ivec)
{
    AES_KEY decrypt_key;
    if(AES_set_decrypt_key(key, 128, &decrypt_key) != 0)
        return ALGORITHM_KEY_ERROR;
    AES_cbc_encrypt(in, out, length, &decrypt_key, ivec, AES_DECRYPT);
    return length;
}
