#!/bin/bash

set -e

TAG="$1"

echo --------------------------
echo ${TAG}TestSerialator
./${TAG}TestSerialator
echo --------------------------
echo ${TAG}TestSerialator2
./${TAG}TestSerialator2
