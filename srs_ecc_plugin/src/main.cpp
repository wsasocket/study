/*
 * main.cpp
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#include "core/Ecc_algorithm.hpp"
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
void  test_ecc_sign_verify();
void test_util();
int main()
{
//   Ecc_Algorithm alg;
//    alg.init_ecc_algorithm(SM_2);
//    std::cout << alg.get_ecc_size() << std::endl;
//    uint8_t buf[128];
//    uint8_t out[128];
//    uint8_t key[16];
//    uint8_t iv[16];
//
//    for(int i = 0; i< 16; i++){
//        key[i] = 0;
//        iv[i] = 0;
//    }
//    for(int i = 0; i< 128; i++){
//        buf[i] = 0;
//        out[i] = 0;
//    }
//    sprintf((char *)buf,"%s","Hello 0123456789 ");
//    int len = strlen((char *)buf);
//    alg.init_symmetric_algorithm(AES_128_CBC);
//    alg.encrypt(key,iv,buf,len,out);
//    std::cout << len << std::endl;
//    for( int i = 0; i < 32; i++)
//        printf("%02x ",buf[i]);
//    printf("\n");
//    for( int i = 0; i < 32; i++)
//        printf("%02x ",out[i]);
//    printf("\n");
//    for(int i = 0; i< 16; i++){
//        //key[i] = 0;
//        iv[i] = 0;
//    }
//    for(int i = 0; i< 128; i++){
//        buf[i] = 0;
//    }
//    alg.decrypt(key,iv,out,len,buf);
//    for( int i = 0; i < 32; i++)
//        printf("%02x ",buf[i]);
//    printf("\n");

    test_ecc_sign_verify();
    test_util();
    return 0;
}

