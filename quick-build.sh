#!/bin/bash -ex

nginx_version=1.19.7
cdir=`cd $(dirname $0); pwd`

(
    cd $cdir/nginx/$nginx_version/nginx-$nginx_version
    cc -c -pipe  -O -Wall -Wextra -Wpointer-arith -Wconditional-uninitialized -Wno-unused-parameter -Wno-deprecated-declarations -Werror -g   -I src/core -I src/event -I src/event/modules -I src/os/unix -I objs -I src/http -I src/http/modules \
                    -o objs/addon/src/ngx_http_periodic_task_module.o \
                    $cdir/.//src/ngx_http_periodic_task_module.c
    cc -c -pipe  -O -Wall -Wextra -Wpointer-arith -Wconditional-uninitialized -Wno-unused-parameter -Wno-deprecated-declarations -Werror -g  -I src/core -I src/event -I src/event/modules -I src/os/unix -I objs \
                    -o objs/ngx_modules.o \
                    objs/ngx_modules.c
    sed -e "s|%%PREFIX%%|$cdir/execs|" \
                    -e "s|%%PID_PATH%%|$cdir/execs/logs/nginx.pid|" \
                    -e "s|%%CONF_PATH%%|$cdir/execs/conf/nginx.conf|" \
                    -e "s|%%ERROR_LOG_PATH%%|$cdir/execs/logs/error.log|" \
                    < man/nginx.8 > objs/nginx.8
    cc -o objs/nginx \
            objs/src/core/nginx.o \
            objs/src/core/ngx_log.o \
            objs/src/core/ngx_palloc.o \
            objs/src/core/ngx_array.o \
            objs/src/core/ngx_list.o \
            objs/src/core/ngx_hash.o \
            objs/src/core/ngx_buf.o \
            objs/src/core/ngx_queue.o \
            objs/src/core/ngx_output_chain.o \
            objs/src/core/ngx_string.o \
            objs/src/core/ngx_parse.o \
            objs/src/core/ngx_parse_time.o \
            objs/src/core/ngx_inet.o \
            objs/src/core/ngx_file.o \
            objs/src/core/ngx_crc32.o \
            objs/src/core/ngx_murmurhash.o \
            objs/src/core/ngx_md5.o \
            objs/src/core/ngx_sha1.o \
            objs/src/core/ngx_rbtree.o \
            objs/src/core/ngx_radix_tree.o \
            objs/src/core/ngx_slab.o \
            objs/src/core/ngx_times.o \
            objs/src/core/ngx_shmtx.o \
            objs/src/core/ngx_connection.o \
            objs/src/core/ngx_cycle.o \
            objs/src/core/ngx_spinlock.o \
            objs/src/core/ngx_rwlock.o \
            objs/src/core/ngx_cpuinfo.o \
            objs/src/core/ngx_conf_file.o \
            objs/src/core/ngx_module.o \
            objs/src/core/ngx_resolver.o \
            objs/src/core/ngx_open_file_cache.o \
            objs/src/core/ngx_crypt.o \
            objs/src/core/ngx_proxy_protocol.o \
            objs/src/core/ngx_syslog.o \
            objs/src/event/ngx_event.o \
            objs/src/event/ngx_event_timer.o \
            objs/src/event/ngx_event_posted.o \
            objs/src/event/ngx_event_accept.o \
            objs/src/event/ngx_event_udp.o \
            objs/src/event/ngx_event_connect.o \
            objs/src/event/ngx_event_pipe.o \
            objs/src/os/unix/ngx_time.o \
            objs/src/os/unix/ngx_errno.o \
            objs/src/os/unix/ngx_alloc.o \
            objs/src/os/unix/ngx_files.o \
            objs/src/os/unix/ngx_socket.o \
            objs/src/os/unix/ngx_recv.o \
            objs/src/os/unix/ngx_readv_chain.o \
            objs/src/os/unix/ngx_udp_recv.o \
            objs/src/os/unix/ngx_send.o \
            objs/src/os/unix/ngx_writev_chain.o \
            objs/src/os/unix/ngx_udp_send.o \
            objs/src/os/unix/ngx_udp_sendmsg_chain.o \
            objs/src/os/unix/ngx_channel.o \
            objs/src/os/unix/ngx_shmem.o \
            objs/src/os/unix/ngx_process.o \
            objs/src/os/unix/ngx_daemon.o \
            objs/src/os/unix/ngx_setaffinity.o \
            objs/src/os/unix/ngx_setproctitle.o \
            objs/src/os/unix/ngx_posix_init.o \
            objs/src/os/unix/ngx_user.o \
            objs/src/os/unix/ngx_dlopen.o \
            objs/src/os/unix/ngx_process_cycle.o \
            objs/src/os/unix/ngx_darwin_init.o \
            objs/src/event/modules/ngx_kqueue_module.o \
            objs/src/os/unix/ngx_darwin_sendfile_chain.o \
            objs/src/core/ngx_regex.o \
            objs/src/http/ngx_http.o \
            objs/src/http/ngx_http_core_module.o \
            objs/src/http/ngx_http_special_response.o \
            objs/src/http/ngx_http_request.o \
            objs/src/http/ngx_http_parse.o \
            objs/src/http/modules/ngx_http_log_module.o \
            objs/src/http/ngx_http_request_body.o \
            objs/src/http/ngx_http_variables.o \
            objs/src/http/ngx_http_script.o \
            objs/src/http/ngx_http_upstream.o \
            objs/src/http/ngx_http_upstream_round_robin.o \
            objs/src/http/ngx_http_file_cache.o \
            objs/src/http/ngx_http_write_filter_module.o \
            objs/src/http/ngx_http_header_filter_module.o \
            objs/src/http/modules/ngx_http_chunked_filter_module.o \
            objs/src/http/modules/ngx_http_range_filter_module.o \
            objs/src/http/modules/ngx_http_gzip_filter_module.o \
            objs/src/http/ngx_http_postpone_filter_module.o \
            objs/src/http/modules/ngx_http_ssi_filter_module.o \
            objs/src/http/modules/ngx_http_charset_filter_module.o \
            objs/src/http/modules/ngx_http_userid_filter_module.o \
            objs/src/http/modules/ngx_http_headers_filter_module.o \
            objs/src/http/ngx_http_copy_filter_module.o \
            objs/src/http/modules/ngx_http_not_modified_filter_module.o \
            objs/src/http/modules/ngx_http_static_module.o \
            objs/src/http/modules/ngx_http_autoindex_module.o \
            objs/src/http/modules/ngx_http_index_module.o \
            objs/src/http/modules/ngx_http_mirror_module.o \
            objs/src/http/modules/ngx_http_try_files_module.o \
            objs/src/http/modules/ngx_http_auth_basic_module.o \
            objs/src/http/modules/ngx_http_access_module.o \
            objs/src/http/modules/ngx_http_limit_conn_module.o \
            objs/src/http/modules/ngx_http_limit_req_module.o \
            objs/src/http/modules/ngx_http_geo_module.o \
            objs/src/http/modules/ngx_http_map_module.o \
            objs/src/http/modules/ngx_http_split_clients_module.o \
            objs/src/http/modules/ngx_http_referer_module.o \
            objs/src/http/modules/ngx_http_rewrite_module.o \
            objs/src/http/modules/ngx_http_proxy_module.o \
            objs/src/http/modules/ngx_http_fastcgi_module.o \
            objs/src/http/modules/ngx_http_uwsgi_module.o \
            objs/src/http/modules/ngx_http_scgi_module.o \
            objs/src/http/modules/ngx_http_memcached_module.o \
            objs/src/http/modules/ngx_http_empty_gif_module.o \
            objs/src/http/modules/ngx_http_browser_module.o \
            objs/src/http/modules/ngx_http_upstream_hash_module.o \
            objs/src/http/modules/ngx_http_upstream_ip_hash_module.o \
            objs/src/http/modules/ngx_http_upstream_least_conn_module.o \
            objs/src/http/modules/ngx_http_upstream_random_module.o \
            objs/src/http/modules/ngx_http_upstream_keepalive_module.o \
            objs/src/http/modules/ngx_http_upstream_zone_module.o \
            objs/addon/src/ngx_http_periodic_task_module.o \
            objs/ngx_modules.o \
            -lpcre -lz

    cp ./objs/nginx $cdir/execs/sbin/nginx
    rm -f $cdir/execs/conf/nginx.conf
    ln -s $cdir/test_nginx.conf $cdir/execs/conf/nginx.conf
)