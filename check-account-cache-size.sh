#!/bin/sh

gdb -iex 'set auto-solib-add off' -ex 'p AccountCache::accounts' -ex detach -ex quit output/server `cat server.pid` | fgrep --colour=always _M_node_count
