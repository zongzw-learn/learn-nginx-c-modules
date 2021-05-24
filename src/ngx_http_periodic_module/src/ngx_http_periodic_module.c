#include <nginx.h>
#include "ngx_http_periodic_module.h"

static ngx_int_t
ngx_http_periodic_init_process(ngx_cycle_t *cycle);

static void
ngx_http_periodic_exit_process(ngx_cycle_t *cycle);

typedef struct {
    ngx_int_t periodic_interval;
    ngx_event_t ev;
} ngx_http_periodic_conf_t;

static char*
ngx_http_set_periodic_interval(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_http_periodic_commands[] = {
    {
        ngx_string("periodic_interval"),
        NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
        // TODO: 使用ngx_conf_set_num_slot，出现异常，待调研。
        ngx_http_set_periodic_interval, 
        0,
        0,
        NULL
    },
    ngx_null_command
};

static void periodic_callback(ngx_event_t *ev);

static void*
ngx_http_periodic_create_main_conf(ngx_conf_t *cf);

static char*
ngx_http_periodic_init_main_conf(ngx_conf_t *cf, void *conf);

static ngx_http_module_t  ngx_http_periodic_module_ctx = {
    NULL,                                    /* preconfiguration */
    NULL,                                    /* postconfiguration */

    ngx_http_periodic_create_main_conf,     /* create main configuration */
    ngx_http_periodic_init_main_conf,       /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_periodic_module = {
    NGX_MODULE_V1,
    &ngx_http_periodic_module_ctx,         /* module context */
    ngx_http_periodic_commands,            /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    ngx_http_periodic_init_process,        /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    ngx_http_periodic_exit_process,        /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static void*
ngx_http_periodic_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_periodic_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_periodic_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, 
        "%s: ngx_http_periodic_conf_t created: %p", __FUNCTION__, conf);
        
    return conf;
}

static char*
ngx_http_set_periodic_interval(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
    ngx_http_periodic_conf_t *p = conf;
    ngx_str_t *value = cf->args->elts;

    p->periodic_interval = ngx_atoi(value[1].data, value[1].len);

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, 
        "%s: directive %V set value %i, conf: %p", 
        __FUNCTION__, &value[0], p->periodic_interval, p);

    return NGX_CONF_OK;
}

char*
ngx_http_periodic_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_periodic_conf_t *pcf = conf;

    pcf->ev.handler = periodic_callback;
    pcf->ev.data = conf;

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, 
        "%s: periodic_interval: %i, conf: %p", 
        __FUNCTION__, pcf->periodic_interval, pcf);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_periodic_init_process(ngx_cycle_t *cycle)
{

    ngx_http_periodic_conf_t *cpcf;
    cpcf = (ngx_http_periodic_conf_t*)ngx_http_cycle_get_module_main_conf(
        cycle, ngx_http_periodic_module);

    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, "%s cpcf: %p", __FUNCTION__, cpcf);

    cpcf->ev.log = cycle->log;

    // The way to get ngx_core_conf_t.
    // ngx_core_conf_t  *ccf;
    // ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
    //                                             ngx_core_module);

    if (cpcf->periodic_interval > 0) {
        ngx_add_timer(&cpcf->ev, cpcf->periodic_interval);
    }

    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, 
        "%s: New timer event is added, with interval: %i", 
        __FUNCTION__, cpcf->periodic_interval);

    return NGX_OK;
}

static void
ngx_http_periodic_exit_process(ngx_cycle_t *cycle)
{
    // ngx_http_periodic_conf_t *cpcf;
    // cpcf = (ngx_http_periodic_conf_t*)ngx_http_cycle_get_module_main_conf(
    //     cycle, ngx_http_periodic_module);

    // cpcf->periodic_interval = 0;
}

static void
periodic_callback(ngx_event_t *ev) 
{
    ngx_http_periodic_conf_t *pcf; 

    pcf = ev->data;
    if (pcf->periodic_interval > 0) {
        ngx_add_timer(ev, pcf->periodic_interval);
    }

    ngx_log_error(NGX_ERROR_INFO, ev->log, 0,
        "%s: Periodic task is triggered: %i", __FUNCTION__, pcf->periodic_interval);
}
