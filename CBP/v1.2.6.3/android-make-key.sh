#!/bin/bash

JAVA_HOME=/usr/lib/jvm/java-6-oracle/bin
KEY_FILE=temp-release-key.keystore
ALIAS=temp-release-key
DNAME="CN=Unknown, OU=Integrated Biometrics, O=Integrated Biometrics, L=Spartanburg, ST=SC, C=US"
KEYPASS=r1A2f4F8L1e6S
STOREPASS=r1A2f4F8L1e6S

$JAVA_HOME/keytool -genkey -v -keystore $KEY_FILE -keypass $KEYPASS -storepass $STOREPASS -dname "$DNAME" -alias $ALIAS -keyalg RSA -keysize 2048 -validity 10000

