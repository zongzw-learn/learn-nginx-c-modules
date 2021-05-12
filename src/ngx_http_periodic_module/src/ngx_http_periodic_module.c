#include <nginx.h>
#include "ngx_http_periodic_module.h"

// TODO: 是否可以删除此全局变量，将其放置在cycle->pool中。
static ngx_event_t periodic_event;

static ngx_int_t
ngx_http_periodic_init_process(ngx_cycle_t *cycle);

static void
ngx_http_periodic_exit_process(ngx_cycle_t *cycle);

typedef struct {
    ngx_int_t periodic_interval;
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
        "%s: directive %V set value %i", 
        __FUNCTION__, &value[0], p->periodic_interval);

    return NGX_CONF_OK;
}

char*
ngx_http_periodic_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_periodic_conf_t *pcf = conf;
    ngx_memzero(&periodic_event, sizeof(ngx_event_t));
    
    periodic_event.handler = periodic_callback;
    periodic_event.data = conf;

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, 
        "%s: periodic_interval: %i", 
        __FUNCTION__, pcf->periodic_interval);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_periodic_init_process(ngx_cycle_t *cycle)
{
    periodic_event.log = cycle->log;
    ngx_http_periodic_conf_t *pcf = periodic_event.data;

    // The way to get ngx_core_conf_t.
    // ngx_core_conf_t  *ccf;
    // ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
    //                                             ngx_core_module);
    // TODO:
    // 此处使用cycle->pool重新申请空间保存*运行时* periodic_event的数据信息。
    // 继续使用ngx_http_periodic_init_main_conf 中的void *conf 会有异常。
    // 即，在申请空间之前，periodic_event.data起到了一个缓存指针的作用.
    // 是否可以通过cycle中直接获取配置信息？
    periodic_event.data = ngx_pcalloc(
        cycle->pool, sizeof(ngx_http_periodic_conf_t));
    if (periodic_event.data == NULL) {
        return NGX_ERROR;
    }
    ngx_memcpy(periodic_event.data, pcf, sizeof(ngx_http_periodic_conf_t));

    // if (pcf->periodic_interval > 0) {
        ngx_add_timer(&periodic_event, pcf->periodic_interval);
    // }

    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, 
        "%s: New timer event is added, with interval: %i", 
        __FUNCTION__, pcf->periodic_interval);

    return NGX_OK;
}

static void
ngx_http_periodic_exit_process(ngx_cycle_t *cycle)
{
    // TODO:
    // 因为采用global variable的方式，所以这里不能既清空又设置。
    // 只有放在cycle中
    // ngx_http_periodic_conf_t *pcf = periodic_event.data;
    // pcf->periodic_interval = 0;
}

static void
periodic_callback(ngx_event_t *ev) 
{
    ngx_http_periodic_conf_t *pcf; 

    pcf = ev->data;
    // if (pcf->periodic_interval > 0) {
    //     ngx_add_timer(ev, pcf->periodic_interval);
    // }

    ngx_log_error(NGX_ERROR_INFO, ev->log, 0,
        "%s: Periodic task is triggered: %i", __FUNCTION__, pcf->periodic_interval);
}
