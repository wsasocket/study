/*
 * Ecc_key_base_manager.h
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#ifndef SRC_CORE_ECC_KEY_BASE_MANAGER_H_
#define SRC_CORE_ECC_KEY_BASE_MANAGER_H_
#include<stdint.h>
#include "Ecc_core_define.hpp"
class Ecc_Key_Base_Manager
{
public:
    Ecc_Key_Base_Manager();
    virtual ~Ecc_Key_Base_Manager();

public:
    virtual int generate_key_pair_base() = 0;
    virtual int export_private_key_base() = 0;
    virtual int import_private_key_base() = 0;
    virtual int load_public_key_base() = 0;
    virtual void set_master_key() = 0;

private:
    virtual int save_private_key_base() = 0;
    virtual int load_private_key_base()= 0;
    virtual int save_public_key_base() = 0;

    const uint8_t *get_public_key_base_x(uint32_t index);
    const uint8_t *get_public_key_base_y(uint32_t index);
    const uint8_t *get_private_key_base(uint32_t index);
private:

    uint8_t *private_key_base;
    uint8_t *public_key_base_x;
    uint8_t *public_key_base_y;
    uint8_t master_key[MAX_HASH_BYTE];
};

#endif /* SRC_CORE_ECC_KEY_BASE_MANAGER_H_ */
