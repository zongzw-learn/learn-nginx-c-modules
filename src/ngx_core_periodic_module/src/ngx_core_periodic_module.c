#include <nginx.h>
#include "ngx_core_periodic_module.h"

static ngx_int_t
ngx_core_periodic_init_process(ngx_cycle_t *cycle);

static void
ngx_core_periodic_exit_process(ngx_cycle_t *cycle);

typedef struct {
    ngx_int_t periodic_interval;
    ngx_event_t periodic_event;
} ngx_core_periodic_conf_t;

static char*
ngx_core_set_periodic_interval(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t ngx_core_periodic_commands[] = {
    {
        ngx_string("core_periodic_interval"),
        NGX_MAIN_CONF|NGX_CONF_TAKE1,
        // TODO: 使用ngx_conf_set_num_slot，出现异常，待调研。
        ngx_core_set_periodic_interval, 
        0,
        0,
        NULL
    },
    ngx_null_command
};

static void periodic_callback(ngx_event_t *ev);

static void*
ngx_core_periodic_create_conf(ngx_cycle_t *cycle);

static char*
ngx_core_periodic_init_conf(ngx_cycle_t *cycle, void *conf);

static ngx_core_module_t  ngx_core_periodic_module_ctx = {
    ngx_string("core_periodic"),
    ngx_core_periodic_create_conf,
    ngx_core_periodic_init_conf
};

ngx_module_t  ngx_core_periodic_module = {
    NGX_MODULE_V1,
    &ngx_core_periodic_module_ctx,         /* module context */
    ngx_core_periodic_commands,            /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    ngx_core_periodic_init_process,        /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    ngx_core_periodic_exit_process,        /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};

static void*
ngx_core_periodic_create_conf(ngx_cycle_t *cycle)
{
    ngx_core_periodic_conf_t *conf;

    conf = ngx_pcalloc(cycle->pool, sizeof(ngx_core_periodic_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, 
        "%s: cpcf created: %p", __FUNCTION__, conf);
        
    return conf;
}

static char*
ngx_core_periodic_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_core_periodic_conf_t *cpcf = conf;
    
    cpcf->periodic_event.handler = periodic_callback;
    cpcf->periodic_event.data = conf;
    cpcf->periodic_event.log = cycle->log;

    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, 
        "%s: %p periodic_interval: %i", 
        __FUNCTION__, cpcf, cpcf->periodic_interval);

    return NGX_CONF_OK;
}

static char*
ngx_core_set_periodic_interval(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
    ngx_core_periodic_conf_t *p = *(ngx_core_periodic_conf_t**)conf;
    ngx_str_t *value = cf->args->elts;

    p->periodic_interval = ngx_atoi(value[1].data, value[1].len);

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, 
        "%s: directive %V set value %i @%p", 
        __FUNCTION__, &value[0], p->periodic_interval, p);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_core_periodic_init_process(ngx_cycle_t *cycle)
{
    ngx_core_periodic_conf_t *cpcf;
    cpcf = (ngx_core_periodic_conf_t*)ngx_get_conf(cycle->conf_ctx, ngx_core_periodic_module);

    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, 
        "%s get cpcf from cycle: %p, interval: %i",
        __FUNCTION__, cpcf, cpcf->periodic_interval);

    ngx_add_timer(&cpcf->periodic_event, cpcf->periodic_interval);
    
    return NGX_OK;
}

static void
ngx_core_periodic_exit_process(ngx_cycle_t *cycle)
{
}

static void
periodic_callback(ngx_event_t *ev) 
{
    // ngx_core_periodic_conf_t *cpcf; 

    // cpcf = ev->data;
    // if (cpcf->periodic_interval > 0) {
    //     ngx_add_timer(ev, cpcf->periodic_interval);
    // }

    // TODO: 为什么这里始终调用不到！
    ngx_log_error(NGX_ERROR_INFO, ev->log, 0,
        "%s: Periodic task is triggered, conf.", __FUNCTION__);
}
