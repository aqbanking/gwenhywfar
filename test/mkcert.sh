#!/bin/sh

NAME="$1"
DAYS="$2"
if test -z "${NAME}"; then
  echo "$0 SERVERNAME [DAYS]"
  exit 1
fi
openssl genrsa -des3 -out ${NAME}.key 1024
openssl rsa -in ${NAME}.key -out ${NAME}.pem
openssl req -new -key ${NAME}.pem -out ${NAME}.csr
if test -n "${DAYS}"; then
 openssl x509 -req -days ${DAYS} -in ${NAME}.csr -signkey ${NAME}.pem -out ${NAME}.crt
else
 openssl x509 -req -in ${NAME}.csr -signkey ${NAME}.pem -out ${NAME}.crt
fi
openssl pkcs12 -export -in ${NAME}.crt -inkey ${NAME}.pem -out ${NAME}.p12 -name "${NAME}"

echo "Certificate and private keys created as"
echo "- ${NAME}.key (private key with passphrase)"
echo "- ${NAME}.pem (private key without passphrase)"
echo "- ${NAME}.crt (certificate)"
echo "- ${NAME}.p12 (certificate in PKCS#12 format)"
