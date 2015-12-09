# weltfrieden

layering and control fragment shaders via osc.

## Setup

**weltfrieden** is experimental software and still has to be built from source. Please make sure you have the needed dependencies for your platform installed.

### Depencendies

#### OSX

```shell
brew install glfw3
```

#### Linux

```shell
apt-get install xorg-dev libglu1-mesa-dev cmake automake xutils-dev libegl1-mesa-dev libtool
```

Install `glfw3` from source:

```shell
git clone https://github.com/glfw/glfw
mkdir -p glfw/build
cd glfw/build
cmake ..
sudo cmake install
```

Install `epoxy` from source:

```shell
git clone https://github.com/anholt/libepoxy
cd libepoxy
./autogen.sh
make
sudo make install
```

### Installation

```shell
git clone https://github.com/fortmeier/weltfrieden
cd weltfrieden
make
./weltfrieden -w 512 -h 512 --cache
```
