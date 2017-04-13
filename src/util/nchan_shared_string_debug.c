#include "nchan_shared_string_debug.h"
#include <assert.h>

#ifndef container_of
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

nchan_shared_string_debug_t *shstring_debug_init(void) {
  int                          i;
  nchan_shared_string_debug_t *shs;
  nchan_shared_string_debug_page_t *cur;
  shs = mmap(NULL, sizeof(shs), PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
  ngx_memzero(shs, sizeof(*shs));
  
  for(i=0; i<NCHAN_SHARED_STRING_SLOTS; i++) {
    cur = mmap(NULL, ngx_pagesize, PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);
    if(shs->free) {
      shs->free->prev = cur;
    }
    cur->next = shs->free;
    cur->prev = NULL;
    cur->str.len = 0;
    cur->str.data = &cur->data;
    cur->reserved = 0;
    shs->free = cur;
  }
  
  ngx_shmtx_create(&shs->mutex, &shs->lock, (u_char *)"shstring debug");
  
  for(cur = shs->free; cur != NULL; cur = cur->next) {
    mprotect(cur, ngx_pagesize, PROT_READ);
  }
  mprotect(shs, sizeof(*shs), PROT_READ);
  
  return shs;
}

ngx_str_t *nchan_shared_string_debug_store(nchan_shared_string_debug_t *shs, ngx_str_t *str_in) {
  ngx_str_t                        *str = NULL;
  nchan_shared_string_debug_page_t *cur;
  
  mprotect(shs, sizeof(*shs), PROT_READ|PROT_WRITE);
  ngx_shmtx_lock(&shs->mutex);
  
  if(shs->free) {
    cur = shs->free;
    mprotect(cur, ngx_pagesize, PROT_READ|PROT_WRITE);
    if(cur->next) {
      mprotect(cur->next, ngx_pagesize, PROT_READ|PROT_WRITE);
      cur->next->prev=NULL;
      mprotect(cur->next, ngx_pagesize, PROT_READ);
      shs->free = cur->next;
    }
    
    if(shs->used) {
      mprotect(shs->used, ngx_pagesize, PROT_READ|PROT_WRITE);
      shs->used->prev = cur;
      mprotect(shs->used, ngx_pagesize, PROT_READ);
    }
    cur->next = shs->used;
    shs->used = cur;
    
    cur->reserved = 1;
    cur->str.len = str_in->len;
    ngx_memcpy(cur->str.data, str_in->data, str_in->len);
    
    mprotect(cur, ngx_pagesize, PROT_READ);
    
    str = &cur->str;
  }
  
  ngx_shmtx_unlock(&shs->mutex);
  mprotect(shs, sizeof(*shs), PROT_READ);
  return str;
}

ngx_int_t nchan_shared_string_debug_clear(nchan_shared_string_debug_t *shs, ngx_str_t *str) {
  nchan_shared_string_debug_page_t *cur;
  cur = container_of(str, nchan_shared_string_debug_page_t, str);
  assert(cur->reserved == 1);
  
  mprotect(shs, sizeof(*shs), PROT_READ|PROT_WRITE);
  ngx_shmtx_lock(&shs->mutex);
  
  mprotect(cur, ngx_pagesize, PROT_READ|PROT_WRITE);
  
  if(cur->prev) {
    mprotect(cur->prev, ngx_pagesize, PROT_READ|PROT_WRITE);
    cur->prev->next = cur->next;
    mprotect(cur->prev, ngx_pagesize, PROT_READ);
  }
  if(cur->next) {
    mprotect(cur->next, ngx_pagesize, PROT_READ|PROT_WRITE);
    cur->next->prev = cur->prev;
    mprotect(cur->next, ngx_pagesize, PROT_READ);
  }
  
  if(shs->used == cur) {
    shs->used = cur->next;
  }
  
  if(shs->free) {
    mprotect(shs->free, ngx_pagesize, PROT_READ|PROT_WRITE);
    shs->free->prev = cur;
    mprotect(shs->free, ngx_pagesize, PROT_READ);
  }
  cur->next = shs->free;
  cur->prev = NULL;
  cur->reserved = 0;
  shs->free = cur;
  
  mprotect(cur, ngx_pagesize, PROT_READ);
  
  ngx_shmtx_unlock(&shs->mutex);
  mprotect(shs, sizeof(*shs), PROT_READ);
  
  return NGX_OK;
}
