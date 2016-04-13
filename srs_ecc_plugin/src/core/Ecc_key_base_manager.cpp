/*
 * Ecc_key_base_manager.cpp
 *
 *  Created on: 2016年4月9日
 *      Author: james
 */

#include "Ecc_key_base_manager.hpp"
#include "Ecc_core_define.hpp"
#include <stdio.h>
Ecc_Key_Base_Manager::Ecc_Key_Base_Manager()
{
    // TODO Auto-generated constructor stub
    private_key_base = NULL;
    public_key_base_x = new uint8_t[PUBLIC_KEY_BYTES * PUBLIC_KEY_BASE_SIZE];
    public_key_base_y = new uint8_t[PUBLIC_KEY_BYTES * PUBLIC_KEY_BASE_SIZE];
}

Ecc_Key_Base_Manager::~Ecc_Key_Base_Manager()
{
    // TODO Auto-generated destructor stub

    delete[] public_key_base_x;
    delete[] public_key_base_y;
}

const uint8_t *Ecc_Key_Base_Manager::get_private_key_base(uint32_t index)
{
    if(index >= PUBLIC_KEY_BASE_SIZE){
        return NULL;
    }
    return &private_key_base[index << 5];
}

const uint8_t *Ecc_Key_Base_Manager::get_public_key_base_x(uint32_t index)
{
    if(index >= PUBLIC_KEY_BASE_SIZE){
        return NULL;
    }
    return &public_key_base_x[index << 5];
}

const uint8_t *Ecc_Key_Base_Manager::get_public_key_base_y(uint32_t index)
{
    if(index >= PUBLIC_KEY_BASE_SIZE){
        return NULL;
    }
    return &public_key_base_y[index << 5];
}
