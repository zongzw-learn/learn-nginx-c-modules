#!/bin/bash

cdir=`cd $(dirname $0); pwd`
nginx_version=1.19.7

nginx-build \
    -v $nginx_version \
    -d $cdir \
    --add-module=$cdir \
    --with-debug \
    --prefix=$cdir/execs


(
    cd $cdir/nginx/$nginx_version/nginx-$nginx_version
    make install
    rm -f $cdir/execs/conf/nginx.conf
    ln -s $cdir/test_nginx.conf $cdir/execs/conf/nginx.conf
)
