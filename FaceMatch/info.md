2011-2017 (C) FaceMatch@NIH.gov: FaceMatch-2 library project directory

This folder provides the development environment for the cross-platform FaceMatch (FM) library with its command-line interface (CLI) modules, supporting tools, utilities, documents and data. FM is implemented by [NIH/NLM/CEB](https://ceb.nlm.nih.gov) FaceMatch team in portable [C++11](https://isocpp.org/wiki/faq/cpp11). FM library source code is located in the folder `common`, and its binaries are compiled in the folder `bin` as `common.dll` (for Windows) or `common.so` (for Linux). The important solution files:

- FaceMatch.sln: Microsoft Visual Studio (MSVS) solution file
- Makefile: UNIX-based library/samples build file
- FaceMatch.cbp: cross-platform Code::Base project file with its FaceMatch.layout
- TODO: summary of high-level R&D tasks

## Dependencies
FM depends on [OpenCV](http://opencv.org)-2.4.x, [OpenMP](http://www.openmp.org/)-2 (typical part of C++ development tools) and optionally on [CUDA](https://developer.nvidia.com/)-8.x SDK. It is recommended to build OpenCV from [source](http://opencv.org/downloads.html), as that would insure optimal performance on a given system. One could also clone OpenCV code [repository](https://github.com/opencv/) to work with the appropriate tag or with the master branch. Building the [contributed](https://github.com/opencv/opencv_contrib) modules is also required because of the patented descriptors (e.g. SIFT) and deep neural network modules. Note: OpenMP and CUDA SDK should be already installed and configured.

FaceMatch team maintains a set of pre-built binaries in the project's shared folder `fmproject/Tools/OpenCV/`. To save time one may consider de-compressing the latest appropriate archive to a local OpenCV home folder (e.g. `~/OpenCV/2.x`) and set up the environment to point to the appropriate places there, i.e.
`LD_LIBRARY_PATH` should contain `~/OpenCV/2.x/lib`, `PKG_CONFIG_PATH` should contain `~/OpenCV/2.x/lib/pkgconfig`, and `~/OpenCV/2.x/lib/pkgconfig/opencv.pc` should exist (being renamed from opencv.pc, if needed).

When building OpenCV from sources, one needs a recent version of [cmake](https://cmake.org/) facility to configure and generate OpenCV solution files. We recommend using `cmake-gui` or `ccmake` application for easier configuration. Consider the following steps:

- clone/download OpenCV [source](https://github.com/opencv/opencv) and [contributions](https://github.com/opencv/opencv_contrib) using the latest tags
- organize them in sibling folders under, e.g. OpenCV/2.x, e.g.

	- Source: library core
	- Contrib: contribution modules
	- Release: release build directory

- use `cmake-gui` or `ccmake` to configure and generate OpenCV solutions, with the defaults set, and the following recommended options:

	- BUILD_EXAMPLES=ON
	- BUILD_opencv_nonfree=ON
	- CMAKE_INSTALL_PREFIX=~/OpenCV/2.x
	- CUDA_ARCH_BIN=3.0 3.5
	- CUDA_FAST_MATH=ON
	- ENABLE_FAST_MATH=ON
	- ENABLE_SS*=ON
	- OPENCV_EXTRA_MODULES_PATH=OpenCV/2.x/Contrib/modules
	- WITH_CUBLAS=ON
	- WITH_OPENMP=ON

- click [configure] (may need multiple iterations) and [generate] to produce OpenCV solutions
- build OpenCV library, e.g. under Linux run `make -j8`
- test OpenCV using tools in bin, e.g. run `performance_gpu` to see benefits of GPU versus CPU
- install the library, e.g. under Linux run `make install`, while under Windows copy `bin`, `lib`, and `include` folders to the OpenCV installation folder

The environment should be augmented as described above.

# Building
Compiling and installation assumes that all dependencies (OpenCV, OpenMP and CUDA) have been properly installed and configured, their headers/binaries are on the respective include/library/path environment variables for the compiler/loader to find them. Running the FM unit tests relies on (a symbolic link to) the Data/CEB/HEPL folder, containing some test images, which can be acquired (or pointed to) at the FMProject network share under the Data folder.
Set setting `JAVA_HOME` to the JDK home folder to properly build Java binding libraries use by ImageListBrowser.

## on Windows
One needs to set OpenCVHome to the OpenCV home directory. The solution file should be opened in MS Visual Studio (MSVS) and re-built using the desired configuration (Release, Debug or Profile). To run the unit tests, at the command prompt in the solution directory type cd bin, then runTests.NetData.bat, or use runTests.bat, if Data/HEPL exists in the solution directory.

## on Linux
We recommend appending `.:bin` to `PATH` and `LD_LIBRARY_PATH`. To build the library, execute `make -jN`, where N is the desired number of CPU cores to build make sub-targets in parallel. The default build is using neutral compiler options. To compile a performance optimized version, call run `make -j release`. For a debug version, use `make -j debug`. For timing/profiling, use `make -j profile`. To clean the project use `make clean`, and to remove all generated binaries for all build options, use `make cleanALL`. To clean and build use `make rebuild`. To test the FaceMatch library, use `make -jN test`. In case of preferring {GPU|CPU}-based execution, run `make -j test{GPU|CPU}`.

# Installation
FaceMatch library is installed by copying the bin folder to the desired location and setting the appropriate environment variables for the system to find the necessary binaries. See README files in the individual sub-directories for more details on those folders.

# Functionality
Latest updates left core FM.lib functionality in-tact, while introducing several improvements, now assuming dependency on OpenCV-2.4.13 compiled with CUDA-8. Windows solution and projects have been converted to MS Visual Studio 2015. Doxygen documentation has been updated [accordingly](file://lhcdevfiler/FMProject/Documents/FM.lib.doc.html/index.html)

## face detection
- updated face/profile/landmark base (Viola-Jones) detection models under FaceMatch/bin/FFModels
- correct face region rectangle estimate from the landmarks arrangement
- correct ordering (w.r.t. image center) of face regions and landmarks in presence of other attributes
- more flexible FaceFinder::gotFaces() now accounting for basic faces when cascading

## face matching
- support per-face region multi-file descriptor index (.mdx) handling for FaceMatch2
- correct query results text serialization input/output (I/O) and more elaborate query output
- query results format option -qrf {XML|YML|YAML} for ImageMatcher CLI
- better conversion/normalization of non-standard 4-channel png images, e.g. japan.cartoon.png

# Modifications
Significant changes since the last tag:

- corrected GPU-based face detection to ensure practical speedup compared to the CPU-based detection
- getVersion: FaceMatch library version as Epoch.Major.Minor.YYYYMMDDtag, where Epoch.Major.Minor is OpenCV version, and YYYYMMDDtag is the date of a tag
- ImageMatcher::load(NdxFN): support for batch index loading, where individual index files are listed new-line delimited in NdxFN

More details can be found in the version control log.
