#!/bin/bash -e
# Copyright 2018 Miklos Vajna. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CWD=$PWD

# Root CA.
export SSLPASS=odfsig
rm -rf ca
mkdir ca
cd ca
mkdir certs crl newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
sed "s|@ROOT@|$CWD|g" $CWD/templates/root.cnf > openssl.cnf
openssl genrsa -aes256 -out private/ca.key.pem -passout env:SSLPASS 4096
chmod 400 private/ca.key.pem

openssl req -config openssl.cnf -key private/ca.key.pem -new -x509 -days 36500 \
    -sha256 -extensions v3_ca -out certs/ca.cert.pem -passin env:SSLPASS -subj \
    '/C=HU/ST=Budapest/O=odfsig test/CN=odfsig test root ca'
chmod 444 certs/ca.cert.pem

# Intermediate CA.
mkdir intermediate
cd intermediate
mkdir certs crl csr newcerts private
chmod 700 private
touch index.txt
echo 1000 > serial
echo 1000 > crlnumber
sed "s|@ROOT@|$CWD|g" $CWD/templates/intermediate.cnf > openssl.cnf
cd -
openssl genrsa -aes256 -out intermediate/private/intermediate.key.pem -passout env:SSLPASS 4096
chmod 400 intermediate/private/intermediate.key.pem

openssl req -config intermediate/openssl.cnf -new -sha256 -key \
    intermediate/private/intermediate.key.pem -out \
    intermediate/csr/intermediate.csr.pem -passin env:SSLPASS -subj \
    '/C=HU/ST=Budapest/O=odfsig test/CN=odfsig intermediate root ca'
openssl ca -batch -config openssl.cnf -extensions v3_intermediate_ca -days \
    36500 -notext -md sha256 -in intermediate/csr/intermediate.csr.pem -passin \
    env:SSLPASS -out intermediate/certs/intermediate.cert.pem
chmod 444 intermediate/certs/intermediate.cert.pem

cat intermediate/certs/intermediate.cert.pem certs/ca.cert.pem > intermediate/certs/ca-chain.cert.pem
chmod 444 intermediate/certs/ca-chain.cert.pem

# Signing keys.
for i in alice bob
do
    openssl genrsa -aes256 -out \
        intermediate/private/example-odfsig-$i.key.pem -passout \
        env:SSLPASS 2048
    chmod 400 intermediate/private/example-odfsig-$i.key.pem

    openssl req -config intermediate/openssl.cnf -key \
        intermediate/private/example-odfsig-$i.key.pem -new -sha256 -out \
        intermediate/csr/example-odfsig-$i.csr.pem -passin env:SSLPASS -subj \
        "/C=HU/ST=Budapest/O=odfsig test/CN=odfsig test example $i"
    openssl ca -batch -config intermediate/openssl.cnf -extensions usr_cert \
        -days 36500 -notext -md sha256 -in \
        intermediate/csr/example-odfsig-$i.csr.pem -passin env:SSLPASS -out \
        intermediate/certs/example-odfsig-$i.cert.pem
    chmod 444 intermediate/certs/example-odfsig-$i.cert.pem

    openssl pkcs12 -export -out \
        ./intermediate/private/example-odfsig-$i.cert.p12 -passout env:SSLPASS \
        -inkey intermediate/private/example-odfsig-$i.key.pem -passin \
        env:SSLPASS -in intermediate/certs/example-odfsig-$i.cert.pem -certfile \
        intermediate/certs/ca-chain.cert.pem \
        -CSP 'Microsoft Enhanced RSA and AES Cryptographic Provider'
done

echo "ca cert: ca/intermediate/certs/ca-chain.cert.pem"
for i in alice bob
do
    echo "signing key: ca/intermediate/private/example-odfsig-$i.cert.p12"
done

# vim:set shiftwidth=4 softtabstop=4 expandtab:
