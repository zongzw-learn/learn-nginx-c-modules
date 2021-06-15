#include <nginx.h>
#include "ngx_http_subrequest_module.h"

static char*
subrequest_directive(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_subrequest_commands[] = {
    {
        ngx_string("combine_subrequest"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        subrequest_directive,
        0,
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

    NULL,                                    /* create location configuration */
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

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, "url path to subrequest: %V", url);

    return NGX_CONF_OK;
}
