/*
 * ECC_core_define.hpp
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#ifndef SRC_CORE_ECC_CORE_DEFINE_HPP_
#define SRC_CORE_ECC_CORE_DEFINE_HPP_

static const int SIGN_BYTES = 32;
static const int PRIVATE_KEY_BYTES = 32;
static const int PUBLIC_KEY_BYTES = 32;
static const int PRIVATE_KEY_BASE_SIZE = 8192;
static const int PUBLIC_KEY_BASE_SIZE = 8192;
static const int MAX_HASH_BYTE = 32;
static const int RESULT_SUCCESS = 1;
static const int RESULT_ERROR = 0;
static const int ALGORITHM_CALC_ERROR = -1;
static const int ALGORITHM_NOT_INITILIZE = -2;
static const int ALGORITHM_NOT_DEFINE = -3;
static const int ALGORITHM_CALC = -4;
static const int ALGORITHM_KEY_ERROR = -5;
static const int ALGORITHM_ECC_GROUP_INIT_FAIL = -6;
static const int ALGORITH_ECC_SIGN_FAIL = -7;
static const int VERIFY_FAIL = 0;
static const int VERIFY_SUCCESS = 1;
enum CURVE_GROUP_ID
{
    SM_2 = 1, X962_SECG = 415, SECG = 714, RFC3639_1 = 927, RFC3639_2 = 928
};

enum HASH_ALG_ID
{
    SM_3 = 0x10, SHA_256, SHA_1, MD_5
};

enum SYMMETRIC_ALG_ID
{
    SM_4_CBC = 0x20, AES_128_CBC,
};
#endif /* SRC_CORE_ECC_CORE_DEFINE_HPP_ */
