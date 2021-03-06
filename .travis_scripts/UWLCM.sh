#!/usr/bin/env sh
set -e

if [ $# -ne 1 ]; then
  echo "UWLCM.sh accepts exactly one argument"
  exit 1
fi

# install libmpata++
cd libmpdata++/build
sudo make install
cd ../..

# making Python 2 back the default if needed - TODO: support Python3 in libcloudph++
if [[ $PY3DEB != '' ]]; then sudo update-alternatives --remove python /usr/bin/python3; fi
if [[ $PY3DEB != '' ]]; then sudo update-alternatives --install /usr/bin/python python /usr/bin/python2 10; fi

# libcloudph++'s dependencies
#if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo $apt_get_install libboost-python-dev python-numpy; fi
if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo $apt_get_install python-numpy; fi
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then brew install boost-python; fi
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then git clone --depth=1 git://github.com/thrust/thrust.git; fi
if [[ $TRAVIS_OS_NAME == 'osx' ]]; then sudo ln -s `pwd`/thrust/thrust /usr/local/include/thrust; fi

# odeint
if [[ $TRAVIS_OS_NAME == 'linux' ]]; then git clone --depth=1 https://github.com/boostorg/odeint.git; fi # get boost odeint > 1.58
if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo rm -f /usr/include/boost/numeric/odeint.hpp; fi
if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo rm -rf /usr/include/boost/numeric/odeint; fi
if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo ln -s `pwd`/odeint/include/boost/numeric/odeint.hpp /usr/include/boost/numeric/odeint.hpp; fi
if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo ln -s `pwd`/odeint/include/boost/numeric/odeint  /usr/include/boost/numeric/; fi

# newest thrust
git clone --depth=1 git://github.com/thrust/thrust.git;
sudo ln -s `pwd`/thrust/thrust /usr/local/include/thrust;

# libcloudph++ 
git clone --depth=1 git://github.com/igfuw/libcloudphxx.git
cd libcloudphxx
mkdir build 
cd build
# RelWithDebInfo = Release with asserts
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../ 
make 
sudo make install
cd ../..

# UWLCM
# if [[ $TRAVIS_OS_NAME == 'linux' ]]; then sudo $apt_get_install libboost-program-options-dev; fi
git clone --depth=1 git://github.com/igfuw/UWLCM.git
cd UWLCM
. .travis_scripts/$1.sh
cd ..
set +e # see https://github.com/travis-ci/travis-ci/issues/6522

