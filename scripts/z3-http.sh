#!/usr/bin/env /bin/sh

# pull latest z3-http image
docker pull evdb/z3-http

# run the image
docker run --rm -it -p "${PORT:-8080}:80" evdb/z3-http