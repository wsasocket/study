/*
 * algorithm_utils.cpp
 *
 *  Created on: 2016年4月12日
 *      Author: james
 */

#include "algorithm_utils.hpp"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <string.h>
algorithm_utils::algorithm_utils()
{
    // TODO Auto-generated constructor stub

}

algorithm_utils::~algorithm_utils()
{
    // TODO Auto-generated destructor stub
}


char *algorithm_utils::base64_encoder(const uint8_t * input, int length, bool with_new_line)
{
    BIO * bmem = NULL;
    BIO * b64 = NULL;
    BUF_MEM * bptr = NULL;

    b64 = BIO_new(BIO_f_base64());
    if(!with_new_line) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char * buff =new char[bptr->length + 1];
    memcpy(buff, bptr->data, bptr->length);
    buff[bptr->length] = 0;

    BIO_free_all(b64);

    return buff;
}

uint8_t *algorithm_utils::base64_decoder(char * input, int length, bool with_new_line)
{
    BIO * b64 = NULL;
    BIO * bmem = NULL;

    if(length < 4)
        return NULL;

    uint8_t * buffer = new uint8_t[length];
    memset(buffer, 0, length);

    b64 = BIO_new(BIO_f_base64());
    if(!with_new_line) {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }
    bmem = BIO_new_mem_buf(input, length);
    bmem = BIO_push(b64, bmem);
    BIO_read(bmem, buffer, length);

    BIO_free_all(bmem);

    return buffer;
}

int algorithm_utils::get_base64_decode_len(char *base64)
{
    int len = 0;
    int i = 0;
    char * buf;

    len = strlen(base64);
    if(len < 4)
         return -1;
    buf = new char[len + 1];
    bzero(buf,len + 1);

    len = 0;
    while(base64[i] != 0){
        if(base64[i] == 0x0a || base64[i] == 0x0d){
            i++;
            continue;
        }
        buf[len] = base64[i];
        len++;
        i ++;
    }

    if(buf[len - 1] == '=' && buf[len - 2] == '='){
        delete [] buf;
        return  (((len - 4) * 6) >> 3) + 1;
    }

    if(buf[len - 1] == '=' && buf[len - 2] != '='){
        delete [] buf;
        return  (((len - 4) * 6) >> 3) + 2;
    }
    if(buf[len - 1] != '=' && buf[len - 2] != '='){
        delete [] buf;
        return  ((len - 4) * 6) >> 3;
    }
    delete [] buf;
    return -1;
}

uint8_t *algorithm_utils::kdf_with_md5(const uint8_t *msg, size_t msg_len, size_t key_len)
{
    uint8_t * digests, *p;
    uint8_t * M;
    uint32_t counter = 1;
    int hash_len = MD5_DIGEST_LENGTH;
    MD5_CTX ctx;

    digests = new uint8_t[(key_len+hash_len)+1];
    M = new uint8_t[(msg_len + 4 + 1)];
    memset(M, 0, msg_len + 4);
    memcpy(M, msg, msg_len);

    p = digests;
    do
    {
        M[msg_len] = (uint8_t)(counter >> 24);
        M[msg_len + 1] = (uint8_t)(counter >> 16);
        M[msg_len + 2] = (uint8_t)(counter >> 8);
        M[msg_len + 3] = (uint8_t)counter;
        MD5_Init(&ctx);
        MD5_Update(&ctx,M,msg_len + 4);
        MD5_Final(p, &ctx);
        counter ++;
        p += hash_len;
    } while ((p - digests) < key_len);

    delete []M;
    return digests;
}

uint8_t *algorithm_utils::kdf_with_sha256(const uint8_t *msg, size_t msg_len, size_t key_len)
{
    uint8_t * digests, *p;
    uint8_t * M;
    uint32_t counter = 1;
    int hash_len = SHA256_DIGEST_LENGTH;
    SHA256_CTX ctx;

    digests = new uint8_t[(key_len+hash_len)+1];
    M = new uint8_t[(msg_len + 4 + 1)];
    memset(M, 0, msg_len + 4);
    memcpy(M, msg, msg_len);

    p = digests;
    do
    {
        M[msg_len] = (uint8_t)(counter >> 24);
        M[msg_len + 1] = (uint8_t)(counter >> 16);
        M[msg_len + 2] = (uint8_t)(counter >> 8);
        M[msg_len + 3] = (uint8_t)counter;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx,M,msg_len + 4);
        SHA256_Final(p, &ctx);
        counter ++;
        p += hash_len;
    } while ((p - digests) < key_len);

    delete []M;
    return digests;
}

