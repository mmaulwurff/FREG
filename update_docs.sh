#!/bin/bash

doxygen Doxyfile &&
optipng *png search/*png;
git commit -am 'docs update';
