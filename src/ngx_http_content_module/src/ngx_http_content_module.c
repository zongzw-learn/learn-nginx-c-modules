#include <nginx.h>
#include "ngx_http_content_module.h"


static char*
ngx_simple_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t
ngx_simple_response_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_content_commands[] = {
    {
        ngx_string("simple_response"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_simple_response,
        0,
        0,
        NULL
    },

    ngx_null_command
};

static ngx_http_module_t  ngx_http_content_module_ctx = {
    NULL,                                    /* preconfiguration */
    NULL,                                    /* postconfiguration */

    NULL,                                    /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_content_module = {
    NGX_MODULE_V1,
    &ngx_http_content_module_ctx,    /* module context */
    ngx_http_content_commands,       /* module directives */
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

static char *
ngx_simple_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
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

    b->last = ngx_snprintf(b->last, last - b->last,
        "hello c module request/response.\n");

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;
    
    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}
