

#include "key_neg.h"
#include "vpn_def.h"

#include <mbedtls/error.h>

int 
key_neg_make_public( struct key_neg *neg )
{
      if ( neg == NULL ) return ( ERROR );
      
      int ret;
      char err_buf[BUFSIZ];
      const char pers[] = "ecdh";
      memset( neg, 0, sizeof( struct key_neg ) );
      mbedtls_ecdh_init( &neg->ctx );
      mbedtls_ctr_drbg_init( &neg->ctr_drbg );
      mbedtls_entropy_init( &neg->entropy );

    
      if (( ret = mbedtls_ctr_drbg_seed( &neg->ctr_drbg, mbedtls_entropy_func, 
                                    &neg->entropy, (const unsigned char *)pers, 
                                    strlen(pers) ) ) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_ctr_drbg_seed) err_code %d, err_msg: %s\n", ret, err_buf );
            goto exit;
      }
      
      if (( ret =  mbedtls_ecp_group_load( &neg->ctx.grp, MBEDTLS_ECP_DP_CURVE25519 ) ) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_ecp_group_load) err_code %d, err_msg: %s\n", ret, err_buf );
            goto exit;
      }

      if (( ret = mbedtls_ecdh_gen_public( &neg->ctx.grp, &neg->ctx.d, &neg->ctx.Q,
                  mbedtls_ctr_drbg_random, &neg->ctr_drbg ) ) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_ecdh_gen_public) err_code %d, err_msg: %s\n", ret, err_buf );
            goto exit;
      }
      
      if (( ret = mbedtls_mpi_write_binary( &neg->ctx.Q.X, neg->pub_key, 32 )) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_mpi_write_binary) err_code %d, err_msg: %s\n", ret, err_buf );
            goto exit;
      }
      
      ret = ( OK );
      exit:
            mbedtls_ecdh_free( &neg->ctx );
            mbedtls_ctr_drbg_free( &neg->ctr_drbg );
            mbedtls_entropy_free( &neg->entropy );
            return ( ret );
}

int 
key_neg_make_secret( struct key_neg *neg, uint8_t *public_key, size_t pklen )
{
      int ret = ( ERROR );
      char err_buf[BUFSIZ];
      size_t olen;

      if ( neg == NULL) goto exit;
     
      if (( ret = mbedtls_mpi_lset( &neg->ctx.Qp.Z, 1 ) ) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_mpi_lset) err_code %d, err_msg: %s\n", 
                        ret, err_buf );
            goto exit;
      }

      if (( ret = mbedtls_mpi_read_binary( &neg->ctx.Qp.X, public_key, pklen ) ) != 0)
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_mpi_read_binary) err_code %d, err_msg: %s\n", 
                        ret, err_buf );
            goto exit;
      }
      
      if (( ret = mbedtls_ecdh_compute_shared( &neg->ctx.grp, &neg->ctx.z,
                                            &neg->ctx.Qp, &neg->ctx.d,
                                            mbedtls_ctr_drbg_random, &neg->ctr_drbg ) ) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_ecdh_compute_shared) err_code %d, err_msg: %s\n", 
                        ret, err_buf );
            goto exit;
      }

      if (( ret = mbedtls_mpi_write_binary( &neg->ctx.z, neg->sec_key, 32 ) ) != 0 )
      {
            mbedtls_strerror( ret, err_buf, BUFSIZ );
            fprintf( stderr, "[ERROR] (mbedtls_mpi_write_binary) err_code %d, err_msg: %s\n", 
                        ret, err_buf );
            goto exit;
      } 
      ret = (OK);
      exit:
            mbedtls_ecdh_free( &neg->ctx );
            mbedtls_ctr_drbg_free( &neg->ctr_drbg );
            mbedtls_entropy_free( &neg->entropy );
            return ( ret );
}

int 
key_neg_pkt(int id, u_int8_t *pub_key, size_t key_len, char* buffer)
{
      struct vpn_proto_header* header = (struct vpn_proto_header *)malloc(sizeof(struct vpn_proto_header));

      if (header == NULL)
      {
            fprintf(stderr, "[ERROR] Malloc error\n");
            return (ERROR);
      }
      header->code = 0;
      header->type = KEY_NEG;
      header->id = htons(id);
      header->length = htons(sizeof(struct vpn_proto_header) + key_len);
      header->reversed = 0;

      memcpy(buffer, (char*)header, sizeof(struct vpn_proto_header));
      memcpy(buffer + sizeof(struct vpn_proto_header), (char *)pub_key, key_len);
      free(header);
      return (sizeof(struct vpn_proto_header) + key_len);
}

int key_neg_pkt_validate(char *buffer, u_int8_t *pub_key, size_t key_len)
{
      if (pub_key == NULL) return (ERROR);

      struct vpn_proto_header* header = (struct vpn_proto_header *)buffer;

      if (header->type != KEY_NEG) return (ERROR);

      memcpy(pub_key, buffer + sizeof(struct vpn_proto_header), key_len);

      return (OK);
}