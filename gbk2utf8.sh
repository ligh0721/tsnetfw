#!/bin/sh
find $1 -type d -exec mkdir -p utf8/{} \;
find $1 -type f -exec iconv -f GBK -t UTF-8 {} -o utf8/{} \;
