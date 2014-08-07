#!/bin/bash

doxygen Doxyfile &&
git add html/*   &&
optipng html/*png html/search/*png;
sed -i "s/ *$//" html/*html html/*css html/*js &&
git commit -am 'docs update';
