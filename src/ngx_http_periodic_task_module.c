#include <nginx.h>
#include "ngx_http_periodic_task_module.h"

static ngx_int_t
ngx_http_periodic_task_init_process(ngx_cycle_t *cycle);

static ngx_command_t ngx_http_periodic_task_commands[] = {
    ngx_null_command
};

static ngx_http_module_t  ngx_http_periodic_task_module_ctx = {
    NULL,                                    /* preconfiguration */
    NULL,                                    /* postconfiguration */

    NULL,                                    /* create main configuration */
    NULL,                                    /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_periodic_task_module = {
    NGX_MODULE_V1,
    &ngx_http_periodic_task_module_ctx,   /* module context */
    ngx_http_periodic_task_commands,       /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    ngx_http_periodic_task_init_process,   /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_periodic_task_init_process(ngx_cycle_t *cycle)
{
    
    for(int i=0; i<10; i++) {
        ngx_log_error(NGX_ERROR_INFO, cycle->log, i, 
            "ngx_http_periodic_task_init_process Called here");
    }
    
    if (ngx_process != NGX_PROCESS_WORKER) {
        return NGX_OK;
    }

    return NGX_OK;
}
