#ifndef NCHAN_SHARED_STRING_DEBUG
#define NCHAN_SHARED_STRING_DEBUG
#include <nchan_module.h>

#define NCHAN_SHARED_STRING_SLOTS 4096

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
} nchan_shared_string_debug_t;



nchan_shared_string_debug_t *shstring_debug_init(void);

#endif //NCHAN_SHARED_STRING_DEBUG
