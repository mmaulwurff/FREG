#!/bin/bash

OUTPUT="everything/everything.cpp"

> $OUTPUT

cat Head.h >> $OUTPUT

echo "/**\file
 * \brief This file includes all cpp files of project.
 *
 * This file is used for one-unit compilation (unity build) for release.
 * One-unit compilation provides faster compilation, a smaller executable,
 * and (probably) higher level of optimization, as modules know more about
 * contents of each other. */" >> $OUTPUT

function include {
    echo >> $OUTPUT
    ls $1 | sed 's/^/#include "/g' | sed 's/$/"/g' >> $OUTPUT
}

include "*.cpp"

while [[ $# > 0 ]]
do
    include "$1/*.cpp"
    shift
done

