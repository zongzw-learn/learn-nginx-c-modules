#!/bin/bash

cdir=`cd $(dirname $0); pwd`
nginx_version=1.19.7

nginx-build \
    --verbose \
    -v $nginx_version \
    -d $cdir \
    --add-module=$cdir/src/ngx_http_subrequest_module \
    --with-debug \
    --with-threads \
    --prefix=$cdir/execs \
    \
    --with-cc-opt=-O0

    # --add-module=$cdir/src/ngx_http_variable_module \
    # --add-module=$cdir/src/ngx_http_content_module \
    # --add-module=$cdir/src/ngx_http_periodic_module \
    # --add-module=$cdir/src/ngx_http_shared_memory_module \
(
    cd $cdir/nginx/$nginx_version/nginx-$nginx_version
    make install
    rm -f $cdir/execs/conf/nginx.conf
    ln -s $cdir/test_nginx.conf $cdir/execs/conf/nginx.conf
)
