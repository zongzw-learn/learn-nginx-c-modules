#include <nginx.h>
#include "ngx_http_c_module.h"

static ngx_event_t periodic_task_event;
static ngx_int_t periodic_interval1 = 6000;

static ngx_int_t
ngx_http_periodic_task_init_process(ngx_cycle_t *cycle);

typedef struct {
    ngx_int_t periodic_interval;
} ngx_http_periodic_task_conf_t;

static char*
ngx_simple_response(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t
ngx_simple_response_handler(ngx_http_request_t *r);

static void
ngx_http_set_foo_intv(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t
ngx_http_get_foo_intv(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data);

static ngx_http_variable_t ngx_foo_variables[] = {
    {
        ngx_string("foo_intv"),
        ngx_http_set_foo_intv,
        ngx_http_get_foo_intv,
        0,
        NGX_HTTP_VAR_CHANGEABLE|NGX_HTTP_VAR_NOCACHEABLE,
        0
    },
    ngx_http_null_variable
};

static ngx_int_t
ngx_add_module_foo_variables(ngx_conf_t *cf) {

    ngx_http_variable_t *var, *v;

    for (v = ngx_foo_variables; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->set_handler = v->set_handler;
        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}

static ngx_command_t ngx_http_periodic_task_commands[] = {
    {
        ngx_string("simple_response"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
        ngx_simple_response,
        0,
        0,
        NULL
    },
    {
        ngx_string("periodic_interval_zong"),
        NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
        ngx_conf_set_num_slot,
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_periodic_task_conf_t, periodic_interval),
        NULL
    },
    ngx_null_command
};

static void myevent_callback(ngx_event_t *ev);

static void*
ngx_http_periodic_task_create_main_conf(ngx_conf_t *cf);

static char*
ngx_http_periodic_task_init_main_conf(ngx_conf_t *cf, void *conf);

static ngx_int_t init_shm_zone(ngx_shm_zone_t *shm_zone, void *data);

static ngx_http_module_t  ngx_http_periodic_task_module_ctx = {
    ngx_add_module_foo_variables,            /* preconfiguration */
    NULL,                                    /* postconfiguration */

    ngx_http_periodic_task_create_main_conf, /* create main configuration */
    ngx_http_periodic_task_init_main_conf,   /* init main configuration */

    NULL,                                    /* create server configuration */
    NULL,                                    /* merge server configuration */

    NULL,                                    /* create location configuration */
    NULL,                                    /* merge location configuration */
};

ngx_module_t  ngx_http_periodic_task_module = {
    NGX_MODULE_V1,
    &ngx_http_periodic_task_module_ctx,    /* module context */
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

static void*
ngx_http_periodic_task_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_periodic_task_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_periodic_task_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->periodic_interval = 1000;
    return conf;
}

static ngx_int_t
ngx_http_periodic_task_init_process(ngx_cycle_t *cycle)
{
    ngx_log_error(NGX_ERROR_INFO, cycle->log, 0, 
        "zongzw ngx_http_periodic_task_init_process Called here");
    
    if (ngx_process != NGX_PROCESS_WORKER) {
        return NGX_OK;
    }

    ngx_memzero(&periodic_task_event, sizeof(ngx_event_t));

    periodic_task_event.handler = myevent_callback;
    periodic_task_event.log = cycle->log;

    ngx_add_timer(&periodic_task_event, periodic_interval1);

    return NGX_OK;
}

static void
myevent_callback(ngx_event_t *ev) 
{
    ngx_log_error(NGX_ERROR_INFO, ev->log, 0,
        "zongzw timer callback is triggered");

    // periodic_interval *= 2;
    ngx_add_timer(ev, periodic_interval1);
}

char*
ngx_http_periodic_task_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_str_t *name;
    ngx_shm_zone_t *shm_zone;

    name = ngx_palloc(cf->pool, sizeof("foo"));
    ngx_str_set(name, "foo");

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, "name: %V", name);

    ngx_int_t size = 1024 * 1024;


    shm_zone = ngx_shared_memory_add(cf, name, size, &ngx_http_periodic_task_module);
    shm_zone->data = cf->pool;
    shm_zone->init = init_shm_zone;

    ngx_log_error(NGX_ERROR_INFO, cf->log, 0, "shm addr: %p\n", shm_zone);

    return NGX_CONF_OK;
}

static ngx_int_t 
init_shm_zone(ngx_shm_zone_t *shm_zone, void *data)
{
    return NGX_OK;
}

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
    ngx_int_t interval = 2222;
    ngx_int_t size = 1024;

    // ngx_str_t response_content;

    ngx_http_periodic_task_conf_t *conf;

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

    conf = ngx_http_get_module_main_conf(r, ngx_http_periodic_task_module);
    if (conf == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    // ngx_str_set(&response_content, "");
    // response_content.data = ngx_pcalloc(r->pool, 1024);
    interval = conf->periodic_interval;
    b->last = ngx_snprintf(b->last, last - b->last,
        "hello c module request/response %d.\n", interval);
    // response_content.len = p - response_content.data;

    // ngx_memcpy(b, response_content.data, response_content.len);

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


static void
ngx_http_set_foo_intv(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_str_t val;
    ngx_str_t arg_name;

    // ngx_conf_t *cf;
    ngx_str_set(&arg_name, "foo_intv");

    val.len = v->len;
    val.data = v->data;

    ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0, "zongzw set intv here: %V, %p", &val, data);

    // cf = ngx_http_conf_get_module_loc_conf(r, ngx_http_periodic_task_module);

    // ngx_int_t index = ngx_http_get_variable_index(cf, &arg_name);

    // ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0, "zongzw set intv here: %V, %p, index: %V", &val, data, index);

}

static ngx_int_t
ngx_http_get_foo_intv(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) 
{
    ngx_log_error(NGX_ERROR_INFO, r->connection->log, 0, "zongzw get intv here.");

    ngx_str_t rlt = ngx_string("zongzw");
    v->data = rlt.data;
    v->len = rlt.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}