#include <nginx.h>
#include "ngx_http_subrequest_module.h"

typedef struct {
    
} ngx_http_subrequest_ctx_t;

typedef struct {
    ngx_str_t uri;
} ngx_http_subrequest_conf_t;

static char*
subrequest_directive(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t
content_responser(ngx_http_request_t *r) ;

static ngx_int_t 
ngx_http_subrequest_done_hander(ngx_http_request_t *r, void *data, ngx_int_t rc);

static void*
ngx_http_subrequest_create_loc_conf(ngx_conf_t *cf);

static ngx_command_t ngx_http_subrequest_commands[] = {
    {
        ngx_string("combine_subrequest"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        subrequest_directive,
        NGX_HTTP_LOC_CONF_OFFSET,       // 相对于ngx_http_core_conf_t结构的起始位置
        0,
        NULL
    },
    ngx_null_command
};

static ngx_http_module_t  ngx_http_subrequest_module_ctx = {
    NULL,                                    /* preconfiguration */
    NULL,                                    /* postconfiguration */

    NULL,                                    /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    ngx_http_subrequest_create_loc_conf,     /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_subrequest_module = {
    NGX_MODULE_V1,
    &ngx_http_subrequest_module_ctx,       /* module context */
    ngx_http_subrequest_commands,          /* module directives */
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

static char*
subrequest_directive(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t *value = cf->args->elts;
    ngx_str_t *url = &value[1];
    ngx_http_core_loc_conf_t *clcf;
    ngx_http_subrequest_conf_t *hscf = conf;

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, "url path to subrequest: %V", url);
    hscf->uri.data = ngx_pcalloc(cf->pool, url->len);
    if (hscf->uri.data == NULL) {
        return NGX_CONF_ERROR;
    }
    ngx_memcpy(hscf->uri.data, url->data, url->len);
    hscf->uri.len = url->len;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = content_responser;

    return NGX_CONF_OK;
}

static ngx_int_t
content_responser(ngx_http_request_t *r) 
{
    ngx_int_t rc;
    // ngx_buf_t *b;
    // ngx_chain_t out;
    // u_char *last;

    if (r->method != NGX_HTTP_GET) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    ngx_http_subrequest_ctx_t *ctx;
    ngx_http_post_subrequest_t *ps;
    ngx_http_subrequest_conf_t *hscf;
    ngx_http_request_t *sr;

    hscf = ngx_http_get_module_loc_conf(r, ngx_http_subrequest_module);

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_subrequest_ctx_t));
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    ps = ngx_pcalloc(r->pool, sizeof(ngx_http_post_subrequest_t));
    if (ps == NULL) {
        return NGX_ERROR;
    }

    ps->handler = ngx_http_subrequest_done_hander;
    ps->data = ctx;

    return ngx_http_subrequest(
        r, &hscf->uri, NULL, &sr, ps, NGX_HTTP_SUBREQUEST_IN_MEMORY);
}

static ngx_int_t
ngx_http_subrequest_done_hander(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
    ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0, 
        "%s is called", __FUNCTION__);

    char  *msg = (char *) data;

    ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0,
                  "done subrequest r:%p msg:%s rc:%i", r, msg, rc);
    // ngx_buf_t *b;
    // ngx_chain_t out;
    // u_char *last;
    
    // r->headers_out.content_type_len = sizeof("text/plain");
    // ngx_str_set(&r->headers_out.content_type, "text/plain");

    // b = ngx_create_temp_buf(r->pool, 1024);
    // if (b == NULL) {
    //     return NGX_HTTP_INTERNAL_SERVER_ERROR;
    // }

    // last = b->last + size;
    // out.buf = b;
    // out.next = NULL;

    // b->last = ngx_snprintf(b->last, last - b->last,
    //     "hello c module request/response.\n");

    // r->headers_out.status = NGX_HTTP_OK;
    // r->headers_out.content_length_n = b->last - b->pos;
    
    // b->last_buf = (r == r->main) ? 1 : 0;
    // b->last_in_chain = 1;

    // rc = ngx_http_send_header(r);

    // if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
    //     return rc;
    // }

    // return ngx_http_output_filter(r, r->out);
    // rc = ngx_http_output_filter(r, r->out);
    // ngx_http_finalize_request(r, rc);
    return rc;
}

static void*
ngx_http_subrequest_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_subrequest_conf_t *hscf;

    hscf = ngx_pcalloc(cf->pool, sizeof(ngx_http_subrequest_conf_t));
    if (hscf == NULL) {
        return NGX_CONF_ERROR;
    }

    return hscf;
}