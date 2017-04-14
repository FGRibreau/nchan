#ifndef NCHAN_SHARED_STRING_DEBUG
#define NCHAN_SHARED_STRING_DEBUG
#include <nchan_module.h>

#define NCHAN_SHARED_STRING_SLOTS 32768

typedef struct nchan_shared_string_debug_page_s nchan_shared_string_debug_page_t;
struct nchan_shared_string_debug_page_s {
 nchan_shared_string_debug_page_t *prev;
 nchan_shared_string_debug_page_t *next;
 ngx_int_t                         reserved;
 ngx_str_t                         str;
 u_char                            data;
}; //nchan_shared_string_debug_page_t

typedef struct {
  ngx_shmtx_sh_t                    lock;
  ngx_shmtx_t                       mutex;
  nchan_shared_string_debug_page_t *used;
  nchan_shared_string_debug_page_t *free;
  ngx_int_t                         used_count;
  ngx_int_t                         free_count;
} nchan_shared_string_debug_t;



nchan_shared_string_debug_t *shstring_debug_init(void);
ngx_str_t *nchan_shared_string_debug_store(nchan_shared_string_debug_t *shs, ngx_str_t *str_in);
ngx_int_t  nchan_shared_string_debug_clear(nchan_shared_string_debug_t *shs, ngx_str_t *str);
#endif //NCHAN_SHARED_STRING_DEBUG
 
