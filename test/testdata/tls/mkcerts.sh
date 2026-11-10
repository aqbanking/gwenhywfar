#!/bin/sh
# Generates the fixed certificates used by "gwentest tlstest".
#
# The files are kept in git, because the test records contain their
# fingerprints. Run this script only to replace all of them.
# Needs OpenSSL 3.4 or later (for "-not_before" and "-not_after").

set -e
cd "$(dirname "$0")"

SUBJ="/C=DE/ST=Niedersachsen/L=Hannover/O=Gwenhywfar Test"

openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out ca.key
openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:2048 -out server.key

# trusted CA
openssl req -x509 -new -key ca.key -sha256 \
  -subj "$SUBJ/OU=Test CA/CN=Gwenhywfar Test CA" \
  -not_before 20260101000000Z -not_after 20991231235959Z \
  -addext "basicConstraints=critical,CA:TRUE" \
  -addext "keyUsage=critical,keyCertSign,cRLSign" \
  -out ca.crt

openssl req -new -key server.key -subj "$SUBJ/OU=Test Server/CN=localhost" -out server.csr

# valid server certificate, issued by the CA
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -sha256 \
  -set_serial 1 -not_before 20260101000000Z -not_after 20991231235959Z \
  -extfile /dev/stdin -out server.crt <<EXT
basicConstraints=CA:FALSE
subjectAltName=DNS:localhost
EXT

# expired server certificate, issued by the CA
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -sha256 \
  -set_serial 2 -not_before 20200101000000Z -not_after 20210101000000Z \
  -extfile /dev/stdin -out expired.crt <<EXT
basicConstraints=CA:FALSE
subjectAltName=DNS:localhost
EXT

# self-signed server certificate, not trusted by the client
openssl req -x509 -key server.key -sha256 \
  -subj "$SUBJ/OU=Self-signed/CN=localhost" \
  -not_before 20260101000000Z -not_after 20991231235959Z \
  -addext "subjectAltName=DNS:localhost" \
  -out selfsigned.crt

rm -f server.csr ca.key
