This is a simple command-line executable interface to the functionality provided by `dlib`'s `find_candidate_object_locations()` function.

# Dependencies

- `argagg`
- `dlib`
- `libjpeg`
- `libpng`
- `opencv >= 3.0`

On Fedora 25 all of these are available through packages except [`argagg`](https://github.com/vietjtnguyen/argagg). The following should be sufficient (untested):

```sh
sudo dnf install dlib-devel opencv-devel libpng-devel libjpeg-devel
```

# Build

Following instructions only tested on Linux. The options for `readlink` on Mac OS are different.

```sh
git clone https://github.com/vietjtnguyen/argagg.git
git clone https://github.com/vietjtnguyen/object-proposals.git
cd object-proposals
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$(readlink -f ../argagg/include) ..
make
./bin/find-object-proposals --help
```

# Usage

```
Usage: ./bin/find-object-proposals INPUT

    -h, --help
        show this help message
    -k
        comma separated list of k values
    -m, --min-size
        minimum size of proposal (default: 20)
    --viz
        visualization output path
    --video
        treat the input as a video
    -s, --start
        start frame when processing a video (default: 0)
    -e, --end
        end frame when processing a video (default: -1, end of video)
    -f, --viz-fps
        if visualization and video is enabled then this option specifics the output frame rate of the visualization video (default: 12)
    -c, --viz-codec
        if visualization and video is enabled then this option specifies the output codec of the visualization video using a FOURCC code (default: same codec as input video)
```
