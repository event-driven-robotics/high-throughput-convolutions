# An Efficient Spatial Convolution Implementation for Event Cameras

## Installation

The `Makefile` file contains commands to download the necessary libraries and dependencies.

This project has been built on top of YARP and Event Driven libraries [Robotology](https://github.com/robotology)

***Tested on Ubuntu 20.04***

1. Download the libraries repositores
   - The makefile will download and install the libraries at `~/libraries` folder.
   - You can set the path for the libraries' code and installation by modifying `PROJECT_DIR` and `INSTALL_DIR` at lines 1 and 2 of `Makefile`.

```sh
make sources
```

2. Compile the libraries
```sh
make
```

3. ***Attention***: dependencies might occur depending on your system
   - Try installing one library at a time in order
   ```sh
   make install-YCM
   make install-yarp
   make install-event-driven
   ```
   - Some common dependencies can be installed with
     ```sh
     make dependencies
     ```
   - refer to [YARP dependencies](http://www.yarp.it/git-master/dependencies.html) and [Event Driven How to install](https://github.com/robotology/event-driven#how-to-install) for more info.

4. Install the convolutions project
```sh
make install-convolution
```

## Running

0. Add the executables to your path (or in your `.<shell>rc` file)
```sh
export PATH=$PATH:/home/leandro/libraries/install/bin
```

1. Start a yarpserver
```sh
yarp server
```

1. Using `yarpmanager` is a convenient way to manage yarp modules. To start it:
```sh
yarpmanager
```

2. We coded an application with the convolution modules
   1. On yarp manager, go to `file -> open file -> ~/libraries/install/share/event-driven/applications -> convolutions.xml`
   2. You can launch the reference implementation `refConv` and our implementation `liteConv`
   3. Depending on your camera, you might want to use a spatio-temporal filter `vPreProcess`

3. If you are using a camera, set the connection directly on the `connection` panel on `yarpmanager`

4. If you are using recorded data, you can start a `yarpdataplayer` to stream the data to the camera:
```sh
yarpdataplayer
```
   - Go to `file->Open Directory`, navigate ans select the ***folder*** generate
   And then set the connection on the `connection` panel on `yarpmanager`

   - The datasets used in the paper are available in a raw text format at [High Speed and HDR Datasets](http://rpg.ifi.uzh.ch/E2VID.html)
   - The conversion to yarp format can be done with [Binvee Library](https://github.com/event-driven-robotics/bimvee)
   - An example script can be found in the `src/python` root folder of the sequence (should look like `<sequence name>_converted`.

## Logging Results

1. On the `Makefile`, you can edit the `LOG=<v>` flag. `<v=0,1,2>` disables logging, logs computation time data, and logs accuracy data, respectively

## Converting the HDR Dataset
1. Download and extract the [HDR datasets](https://rpg.ifi.uzh.ch/E2VID.html)

2. We use `bimvee` for converting the `.txt` HDR dataset file into yarp format
```sh
pip install bimvee
export DATASETS_PAT=<datasets location>
python src/python/gen3SequencesToYarp.py <sequence_name>
```
 - `<sequence_name>` without the `.txt` extension
 - `<datasets location>` is the folder in which you downloaded and extracted the datasets 
 - E.g.: `python src/python/gen3SequencesToYarp.py hdr_sun`


