#!/bin/sh

function readFile() {
  while read line; do
    nline=`echo $line | sed -e 's!"!\\\\"!g'`
    echo " \"$nline\\n\""
  done
}


echo "static const char *gwen_xml__vsdtypes="
readFile <$1
echo ";"
