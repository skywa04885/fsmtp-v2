#!/bin/bash

# Installs the python dependencies

pip install pyOpenSSL
pip install cassandra-driver

# Installs the C++ dependencies

CMAKE_ARGS="-DCMAKE_INSTALL_PREFIX:PATH=/usr"
MAKE_ARGS="-j 12"

mkdir -p dependencies 

function install {
  cd dependencies

  git clone $1
  cd $2
  mkdir build -p && cd build
  cmake .. $CMAKE_ARGS
  make $MAKE_ARGS && sudo make install

  cd ../../../dependencies
}

install "https://github.com/nickbruun/hayai" "hayai"
install "https://github.com/onqtam/doctest" "doctest"
install "https://github.com/nlohmann/json" "json"
install "https://github.com/pantor/inja" "inja"
install "https://github.com/redis/hiredis" "hiredis"
install "https://github.com/datastax/cpp-driver" "cpp-driver"
install "https://github.com/open-source-parsers/jsoncpp" "jsoncpp"
