#!/bin/bash
# written by Thomas Viehmann, with a slight modification by Martin Preuss
#
set -e
# run in top source dir to correct the misspelling
if [ "$1" == "content" ] && [ "$2" != "" ] ; then
  AFILE=$2
  case "$AFILE" in
    *gwenspell.sh)
      ;;
    *~) 
      ;;
    *.o) 
      ;;
    *.lo) 
      ;;
    *.la) 
      ;;
    *)
      echo "Processing content:" ${AFILE}
      sed 's/gwenhyfwar/gwenhywfar/g' < "${AFILE}" > "${AFILE}.gwenspell.tmp" &&
      mv "${AFILE}.gwenspell.tmp" "${AFILE}"
      ;;
  esac
elif [ "$1" == "filename" ] && [ "$2" != "" ] ; then
  AFILE=$2
  DESTFILE=$(echo ${AFILE} | sed 's/gwenhyfwar/gwenhywfar/g' )
  if [ ${DESTFILE} != ${AFILE} ] ; then
    echo "Moving:" $2
    mv -i $AFILE $DESTFILE ;
  fi 
elif [ "$1" == "" ] ; then
  SELF=${PWD}/$0
  echo "Checking filenames..."
  find -name \*gwenhyfwar\* -and -exec ${SELF} filename '{}' ';'
  echo "Checking file contents..."
  find -type f -and -exec grep -q gwenhyfwar '{}' ';' -and -exec ${SELF} content '{}' ';'
else
  echo "Use without argument to start..."
  exit 1
fi
