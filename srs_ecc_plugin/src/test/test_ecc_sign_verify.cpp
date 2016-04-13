#include "../core/Ecc_algorithm.hpp"
#include "../core/Hash_algorithm.hpp"
#include "../core/Ecc_core_define.hpp"
#include <stdio.h>
#include <string.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
void  test_ecc_sign_verify()
{
    char M[128];
    uint8_t hash[32];
    uint8_t private_key[32];
    uint8_t public_key_x[32];
    uint8_t public_key_y[32];
    uint8_t sign_r[32];
    uint8_t sign_s[32];
    const EC_POINT * point_public_key;
    const BIGNUM * bn_private_key;
    const EC_GROUP * group;
    BIGNUM * bn_public_key_x;
    BIGNUM * bn_public_key_y;
    int len;

    sprintf(M,"%s","Hello world!");
    len = strlen(M);

    EC_KEY * eckey = NULL;
    eckey = EC_KEY_new_by_curve_name(X962_SECG);
    EC_KEY_generate_key(eckey);

    bn_public_key_x = BN_new();
    bn_public_key_y = BN_new();

    bn_private_key = EC_KEY_get0_private_key(eckey);
    point_public_key = EC_KEY_get0_public_key(eckey);
    group = EC_KEY_get0_group(eckey);
    EC_POINT_get_affine_coordinates_GFp(group,point_public_key,bn_public_key_x,bn_public_key_y,NULL);

    int o ;
    o= ((SIGN_BYTES << 3) - BN_num_bits(bn_public_key_x)) >> 3;
    BN_bn2bin(bn_public_key_x,(uint8_t *)(public_key_x + o));
    o = ((SIGN_BYTES << 3) - BN_num_bits(bn_public_key_y)) >> 3;
    BN_bn2bin(bn_public_key_y,(uint8_t *)(public_key_y + o));
    o = ((SIGN_BYTES << 3) - BN_num_bits(bn_private_key)) >> 3;
    BN_bn2bin(bn_private_key,(uint8_t *)(private_key + o));

    Ecc_Algorithm algorithm;
    Hash_algorithm hash_alg;

    hash_alg.init_hash_algorithm(SHA_256);
    algorithm.init_ecc_algorithm(X962_SECG,hash_alg.get_hash_out_size());
    bzero(hash,32);
    hash_alg.hash((uint8_t *)M,len,hash);
    algorithm.ecc_sign(hash,private_key,sign_r,sign_s);

    for(int i = 0;i<32;i++)
        printf("%02x",sign_r[i]);
    printf("\n");

    for(int i = 0;i<32;i++)
        printf("%02x",sign_s[i]);
    printf("\n");
    //sign_s[0] ^= 0xff;
    if(algorithm.ecc_verify(hash,public_key_x,public_key_y,sign_r,sign_s) == VERIFY_SUCCESS)
        printf("Verify success\n");
    else
        printf("Verify fail\n");

    EC_KEY_free(eckey);
    BN_free(bn_public_key_x);
    BN_free(bn_public_key_y);
}
