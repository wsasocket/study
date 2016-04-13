/*
 * EcckeyManager.hpp
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#ifndef SRC_CORE_ECC_KEY_MANAGER_HPP_
#define SRC_CORE_ECC_KEY_MANAGER_HPP_
#include <stdint.h>
#include "Ecc_core_define.hpp"
class Ecc_Key_Manager
{
public:
    Ecc_Key_Manager();
    virtual ~Ecc_Key_Manager();

public:
    virtual int generate_key_pair() = 0;
    virtual int export_private_key() = 0;
    virtual int import_private_key() = 0;
    virtual int load_public_key()= 0;
    virtual int set_PIN_code() = 0;

private:
    virtual int save_private_key() = 0;
    virtual int load_private_key()= 0;
    virtual int save_public_key() = 0;

private:
    uint8_t private_key[PRIVATE_KEY_BYTES];
    uint8_t public_key_x[PUBLIC_KEY_BYTES];
    uint8_t public_key_y[PUBLIC_KEY_BYTES];
    uint8_t pin_code[MAX_HASH_BYTE];

};

#endif /* SRC_CORE_ECC_KEY_MANAGER_HPP_ */
