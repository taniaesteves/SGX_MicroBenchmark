//
// Created by rgmacedo on 15-03-2018.
//

#ifndef __AUTH_ENCRYPTION_H__
#define __AUTH_ENCRYPTION_H__

#include <openssl/ossl_typ.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#define GCM 1
#define CCM 2


int auth_init(int key_size, int iv_size, int tag_size, int operation_mode);

int auth_encode(unsigned char* key, unsigned char* iv, unsigned char* dest, const unsigned char* src, int size, unsigned char* tag);

int auth_decode(unsigned char* key, unsigned char* iv, unsigned char* dest, const unsigned char* src, int size, unsigned char* tag);

int auth_clean();

void auth_handleErrors(void);

int auth_get_cycle_block_size(int origin_size, int is_last_cycle, int mode, int blk_size, int pad_size);

int auth_get_cycle_block_offset(int cycle, int blk_size, int pad_size);

int auth_get_total_decoding_cycles(int size, int blk_size, int pad_size);

int auth_get_encrypted_chunk_size(int encrypted_size, int is_last_cycle, int blk_size, int pad_size);

int auth_get_plaintext_block_offset(int cycle, int blk_size);


#endif