#include <nginx.h>
#include "ngx_http_variable_module.h"

size_t max_str_len = 128;

static ngx_int_t
ngx_add_module_my_variables(ngx_conf_t *cf);

static void
ngx_http_set_my_var(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t
ngx_http_get_my_var(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_http_variable_t ngx_my_variables[] = {
    {
        ngx_string("my_var"),
        ngx_http_set_my_var,
        ngx_http_get_my_var,
        0,
        NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE,
        0
    },
    ngx_http_null_variable
};

static ngx_http_module_t  ngx_http_variable_module_ctx = {
    ngx_add_module_my_variables,             /* preconfiguration */
    NULL,                                    /* postconfiguration */

    NULL,                                    /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_variable_module = {
    NGX_MODULE_V1,
    &ngx_http_variable_module_ctx,         /* module context */
    NULL,                                  /* module directives */
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

static ngx_int_t
ngx_add_module_my_variables(ngx_conf_t *cf) {

    ngx_http_variable_t *var, *v;

    for (v = ngx_my_variables; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        void *p = ngx_pcalloc(cf->pool, sizeof(ngx_str_t));
        if (p == NULL) {
            return NGX_ERROR;
        }
        var->set_handler = v->set_handler;
        var->get_handler = v->get_handler;
        var->data = (uintptr_t)p;
    }

    return NGX_OK;
}

static void
ngx_http_set_my_var(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_str_t val;
    ngx_str_t arg_name;

    ngx_str_set(&arg_name, "my_var");

    val.len = v->len;
    val.data = v->data;
    
    ngx_str_t *p = (ngx_str_t*)data;
    p->data = ngx_pcalloc(r->pool, max_str_len);
    if (p->data == NULL) {
        return;
    }

    p->len = (v->len > max_str_len) ? max_str_len : v->len;
    ngx_memcpy(p->data, v->data, p->len);
    
    ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0, 
        "%s set variable: %V", __FUNCTION__, p);
}

static ngx_int_t
ngx_http_get_my_var(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) 
{
    ngx_str_t *p = (ngx_str_t*) data;

    v->data = p->data;
    v->len = p->len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0, 
        "%s get variable: %s", __FUNCTION__, p);
    return NGX_OK;
}
