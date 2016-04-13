#include "../core/algorithm_utils.hpp"
#include <stdio.h>
#include<string.h>
void test_util()
{
    uint8_t M[128];
    uint8_t *keys;
    char *base64;
    sprintf((char *)M,"%s","Hello World!");
    keys = algorithm_utils::kdf_with_md5(M,strlen((char *)M),256);
    for(int i=0;i<256;i++)
    {
        printf("%02x ",keys[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }
    delete[] keys;
    keys = NULL;
    printf("\n");
    keys = algorithm_utils::kdf_with_sha256(M,strlen((char *)M),256);
    for(int i=0;i<256;i++)
    {
        printf("%02x ",keys[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n");
    base64 = algorithm_utils::base64_encoder(keys,256,true);
    printf("%s\n",base64);
    delete[] keys;
    keys = NULL;
    keys = algorithm_utils::base64_decoder(base64,strlen(base64),true);
    int len = algorithm_utils::get_base64_decode_len(base64);
    printf("original bin data size = %d\n",len);
    for(int i=0;i<len;i++)
    {
        printf("%02x ",keys[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n");
    delete [] base64;
    delete [] keys;
}
