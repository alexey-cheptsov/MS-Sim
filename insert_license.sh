#!/bin/bash

#for f in *.{cpp,h}; do
for f in $(find . -name '*.cpp' -or -name '*.h'); do
  cat LICENSE_HEADER $f > $f.new
  mv $f.new $f
done