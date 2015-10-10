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
apt-get install xorg-dev libglu1-mesa-dev libglew-dev cmake automake xutils-dev libegl1-mesa-dev
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
./weltfrieden
```

run this in `ghci` before using weltfrieden

```haskell
let splashState = Sound.Tidal.Context.state "127.0.0.1" 7772 dirt

let splashSetters getNow = do ss <- splashState
                              return (setter ss, transition getNow ss)

(cps, getNow) <- cpsUtils

(s1,st1) <- splashSetters getNow
(s2,st2) <- splashSetters getNow
(s3,st3) <- splashSetters getNow
```


## Usage

```haskell
s1 $ sound "basic"
```

will flash the screen white then fade every cycle.

You can use all known dirt params and commands with `s1`..`3` and `st1`..`3`.

Special handling occurs on `vowel` which is interpreted as blend mode for each fragment shader layer.

vowel accepts the following chars and maps them to the corresponding GL blend mode.

* `c` SRC_COLOR
* `a` SRC_ALPHA
* `C` DST_COLOR
* `A` DST_ALPHA
* `l` CONSTANT_ALPHA
* `t` CONSTANT_COLOR
* `s` SRC_SATURATE_ALPHA
* `x` ONE_MINUS_SRC_ALPHA
* `y` ONE_MINUS_SRC_COLOR
* `X` ONE_MINUS_DST_ALPHA
* `Y` ONE_MINUS_DST_COLOR

### Shaders

Depending on your graphics card certain sets of shaders are available.
When `weltfrieden` is started, it reports

```shell
...
[INFO] (weltfrieden.c:142) Shading Language Level: 1xx
...
```

Currently a few `1xx` shaders are implemented and a few more `3xx`. Below you'll find a listing of the two sets of shaders.


#### 1xx

##### Backgrounds

- **basic** fading from white to black

more to come soon...

##### Overlays

#### 3xx

They implement at least the following method: `gain`

##### Backgrounds

- **basic** fading from white to black
- **white** plain white, opaque
- **shift** rainbow, define horizontal extents via `begin` and `end`
- **fall** rainbow, define vertical extent via `begin` and `end`

##### Overlays
- **grid** white squares, evenly spaced, define size via `shape` and move via `offset`
- **form** - product of tangent of x and y of frag coord visualized, alternate via `offset` which is added
before computing tangents and `shape` which is a threshold


## Extending

You can simply add more `.frag` files containing fragment shader code and place them in the `shaders` subdirectory.
From then on you can access your new shader via its name, e.g. if you added `shaders/myshader.frag` you can now do:

```haskell
s1 $ sound "myshader"
```

and have it played. (Debugging is currently turned of, so any errors in Shader code will not be shown at runtime
and simply fail - black screen)

Feel free to add a pull request to have them merged into this repository.

## Limitations

- Blending isn't perfect yet
- Ordering of shaders is important, maybe implement `cut` to separate cutgroups, or use concept of weight here to
let certain layers float on top of others
- write fragment shader result to texture, read form texture in next shader, feedback loop and render result texture in the end
