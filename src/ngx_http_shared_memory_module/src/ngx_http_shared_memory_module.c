#include <nginx.h>
#include "ngx_http_shared_memory_module.h"

typedef struct {
    ngx_str_t v_str;
} ngx_http_shared_memory_myvals_t;

typedef struct {
    ngx_shm_zone_t *zone;
    ngx_str_t zone_name;
} ngx_http_shared_memory_conf_t;

static char*
zong_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char*
ngx_get_set_zone_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t
ngx_simple_response_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_shared_memory_commands[] = {
    {
        ngx_string("zong_zone"),
        NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
        zong_zone,
        0,
        0,
        NULL
    },
    {
        ngx_string("get_set_zone_value"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_get_set_zone_value,
        0,
        0,
        NULL
    },
    ngx_null_command
};

static void*
ngx_shared_memory_create_main_conf(ngx_conf_t *cf);

static ngx_http_module_t  ngx_http_shared_memory_module_ctx = {
    NULL,                                    /* preconfiguration */
    NULL,                                    /* postconfiguration */

    ngx_shared_memory_create_main_conf,      /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_shared_memory_module = {
    NGX_MODULE_V1,
    &ngx_http_shared_memory_module_ctx,    /* module context */
    ngx_http_shared_memory_commands,       /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static void*
ngx_shared_memory_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_shared_memory_conf_t *smcf;

    smcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_shared_memory_conf_t));
    if (smcf == NULL) {
        return NULL;
    }
    ngx_log_error(NGX_ERROR_INFO, 
        cf->log, 0, "%s: ngx_http_shared_memory_conf_t created: %p", __FUNCTION__, smcf);

    return smcf;
}


static ngx_int_t
init_zong_zone(ngx_shm_zone_t *zone, void *data) 
{
    ngx_slab_pool_t *shpool = (ngx_slab_pool_t*) zone->shm.addr;

    ngx_http_shared_memory_myvals_t *myvals = ngx_slab_alloc(shpool, sizeof(ngx_http_shared_memory_myvals_t));
    if (myvals == NULL) {
        return NGX_ERROR;
    }

    ngx_str_t tmp;
    ngx_str_set(&tmp, "zongzw-default");
    myvals->v_str.data = ngx_slab_alloc(shpool, tmp.len);
    if (myvals->v_str.data == NULL) {
        return NGX_ERROR;
    }
    ngx_memcpy(myvals->v_str.data, tmp.data, tmp.len);
    myvals->v_str.len = tmp.len;

    shpool->data = myvals;

    return NGX_OK;
}

static char*
zong_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value = cf->args->elts;
    ngx_str_t *name = &value[1];
    ngx_int_t size = 65536;

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, "zone name: %V", name);

    ngx_shm_zone_t *shm_zone = ngx_shared_memory_add(cf, name, size, &ngx_http_shared_memory_module);
    if (shm_zone == NULL) {
        return NGX_CONF_ERROR;
    }

    shm_zone->init = init_zong_zone;

    ngx_http_shared_memory_conf_t *smcf;

    smcf = conf;

    smcf->zone = shm_zone;
    smcf->zone_name.data = ngx_pcalloc(cf->pool, name->len);
    ngx_memcpy(smcf->zone_name.data, name->data, name->len);
    smcf->zone_name.len = name->len;

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, "smcf zone_name: %V, zone %p", &smcf->zone_name, shm_zone);
    return NGX_CONF_OK;
}

static char *
ngx_get_set_zone_value(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_simple_response_handler;

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_simple_response_handler(ngx_http_request_t *r) 
{
    ngx_int_t rc;
    ngx_buf_t *b;
    ngx_chain_t out;
    u_char *last;
    ngx_int_t size = 1024;

    ngx_http_shared_memory_conf_t *smcf;

    if (r->method != NGX_HTTP_GET) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    r->headers_out.content_type_len = sizeof("text/plain");
    ngx_str_set(&r->headers_out.content_type, "text/plain");

    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    last = b->last + size;
    out.buf = b;
    out.next = NULL;

    smcf = ngx_http_get_module_main_conf(r, ngx_http_shared_memory_module);
    ngx_slab_pool_t *shpool = (ngx_slab_pool_t *) smcf->zone->shm.addr;
    ngx_http_shared_memory_myvals_t *myval = shpool->data;
    
    if (r->args.len) {
        ngx_slab_free_locked(shpool, myval->v_str.data);
        void *p = ngx_slab_alloc_locked(shpool, r->args.len);
        ngx_memcpy(p, r->args.data, r->args.len);
        myval->v_str.data = p;
        myval->v_str.len = r->args.len;
        shpool->data = myval;
    }

    b->last = ngx_snprintf(b->last, last - b->last,
        "last request arguments %V.\n", myval);

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;
    
    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;
    // b->last = b->pos + response_content.len;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}
