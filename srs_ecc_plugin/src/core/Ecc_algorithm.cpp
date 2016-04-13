/*
 * Ecc_slgorithm.cpp
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#include "Ecc_algorithm.hpp"
#include<openssl/ecdsa.h>
#include <openssl/bn.h>

#include <string.h>
Ecc_Algorithm::Ecc_Algorithm()
{
    // TODO Auto-generated constructor stub
    ecc_id = -1;
    hash_len = 0;
    group = NULL;
}

Ecc_Algorithm::~Ecc_Algorithm()
{
    // TODO Auto-generated destructor stub
    if(group!=NULL)
        EC_GROUP_free((EC_GROUP *)group);
    group = NULL;
}

int Ecc_Algorithm::init_ecc_algorithm(CURVE_GROUP_ID oid, size_t hash_len)
{
    ecc_id = oid;
    this->hash_len = hash_len;

    group = EC_GROUP_new_by_curve_name(ecc_id);
    if(group == NULL)
        return ALGORITHM_ECC_GROUP_INIT_FAIL;
    return get_ecc_size();
}

int Ecc_Algorithm::get_ecc_size()
{
    return ecc_id == -1 ? -1 : 32;
}

int Ecc_Algorithm::ecc_sign(uint8_t *hash, uint8_t *private_key, uint8_t *sign_r, uint8_t *sign_s)
{
    EC_KEY * eckey = NULL;
    ECDSA_SIG * sig = NULL;
    BIGNUM * bn_private_key = NULL;
    BIGNUM * bn_sign_r = NULL;
    BIGNUM * bn_sign_s = NULL;
    int ret = RESULT_SUCCESS;

    if(group == NULL)
        return ALGORITHM_ECC_GROUP_INIT_FAIL;

    bn_private_key = BN_new();
    bn_sign_r = BN_new();
    bn_sign_s = BN_new();
    BN_bin2bn(private_key, PRIVATE_KEY_BYTES, bn_private_key);

    eckey = EC_KEY_new();
    EC_KEY_set_group(eckey, group);
    EC_KEY_set_private_key(eckey, bn_private_key);

    sig = ECDSA_do_sign(hash, hash_len, eckey);
    if(sig == NULL){
        ret = ALGORITH_ECC_SIGN_FAIL;
        goto error;
    }
    bzero(sign_r, SIGN_BYTES);
    bzero(sign_s, SIGN_BYTES);
    //ECDSA_SIG_get0
    BN_copy(bn_sign_r, sig->r);
    BN_copy(bn_sign_s, sig->s);

    int o;
    o = ((SIGN_BYTES << 3) - BN_num_bits(bn_sign_r)) >> 3;
    BN_bn2bin(bn_sign_r, sign_r + o);
    o = ((SIGN_BYTES << 3) - BN_num_bits(bn_sign_s)) >> 3;
    BN_bn2bin(bn_sign_s, sign_s + o);

    error: EC_KEY_free(eckey);
    ECDSA_SIG_free(sig);
    BN_free(bn_private_key);
    BN_free(bn_sign_r);
    BN_free(bn_sign_s);
    return ret;
}

int Ecc_Algorithm::ecc_verify(uint8_t *origin_hash, uint8_t *public_key_x, uint8_t *public_key_y, uint8_t *sign_r, uint8_t *sign_s)
{
    EC_KEY * eckey = NULL;
    ECDSA_SIG * sig = NULL;
    BIGNUM * bn_public_key_x = NULL;
    BIGNUM * bn_public_key_y = NULL;
    BIGNUM * bn_sign_r = NULL;
    BIGNUM * bn_sign_s = NULL;
    int ret = VERIFY_SUCCESS;

    if(group == NULL)
        return ALGORITHM_ECC_GROUP_INIT_FAIL;

    bn_public_key_x = BN_new();
    bn_public_key_y = BN_new();
    bn_sign_r = BN_new();
    bn_sign_s = BN_new();
    BN_bin2bn(public_key_x, PRIVATE_KEY_BYTES, bn_public_key_x);
    BN_bin2bn(public_key_y, PRIVATE_KEY_BYTES, bn_public_key_y);
    BN_bin2bn(sign_r, PRIVATE_KEY_BYTES, bn_sign_r);
    BN_bin2bn(sign_s, PRIVATE_KEY_BYTES, bn_sign_s);
    eckey = EC_KEY_new();
    EC_KEY_set_group(eckey, group);
    EC_KEY_set_public_key_affine_coordinates(eckey, bn_public_key_x, bn_public_key_y);

    sig = ECDSA_SIG_new();

    BN_copy(sig->r, bn_sign_r);
    BN_copy(sig->s, bn_sign_s);

    ret = ECDSA_do_verify(origin_hash, hash_len, sig, eckey);

    EC_KEY_free(eckey);
    ECDSA_SIG_free(sig);
    BN_free(bn_public_key_x);
    BN_free(bn_public_key_y);
    BN_free(bn_sign_r);
    BN_free(bn_sign_s);
    return ret;
}

int Ecc_Algorithm::ecc_dh_exchange_key(uint8_t *Pub_r_x, uint8_t *Pub_r_y, //peer random number K-point
        uint8_t *Pub_x, uint8_t *Pub_y,  //peer public key
        uint8_t *self_r, //self random number
        uint8_t *self_Pri, // self private key
        uint8_t *shared_key  // result ,for ECC 256,out size is 256 bit
        )
{
    return ALGORITHM_NOT_DEFINE;
}
// for ECC 256,returned result length should increase 64 bytes for package
int Ecc_Algorithm::ecc_encrypt(uint8_t *in, size_t in_length, uint8_t *Private_key, uint8_t *out)
{
    return ALGORITHM_NOT_DEFINE;
}
// see above and think
int Ecc_Algorithm::ecc_decrypt(uint8_t *in, size_t in_length, uint8_t *Private_key, uint8_t *out)
{
    return ALGORITHM_NOT_DEFINE;
}

