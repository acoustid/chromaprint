Chromaprint
===========

Chromaprint is an audio fingerprint library developed for the [AcoustID][acoustid] project.

[acoustid]: https://acoustid.org/

Dependencies
------------

The only require external dependency is a FFT library.
This can be [FFmpeg][ffmpeg], [FFTW3][fftw] or [KissFFT][kissfft] or
the [Accelerate/vDSP framework][vdsp] (macOS).
See the next section for details.

The `fpcalc` utility included in the package needs FFmpeg for audio decoding.

[ffmpeg]: https://www.ffmpeg.org/
[fftw]: http://www.fftw.org/
[kissfft]: https://sourceforge.net/projects/kissfft/
[vdsp]: https://developer.apple.com/reference/accelerate/1652565-vdsp

Installing
----------

The most common way to build Chromaprint is like this:

    $ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TOOLS=ON .
    $ make
    $ sudo make install

This will build Chromaprint as a shared library and also include the `fpcalc`
utility (which is used by MusicBrainz Picard, for example).

See below for other options.

FFT Library
-----------

Chromaprint can use multiple FFT libraries -- FFmpeg, FFTW3, KissFFT and vDSP.
FFmpeg is preferred on all systems except for macOS, where you should use
the standard vDSP framework. These are the fastest options.

FFTW3 can be also used, but this library is released under the GPL
license, which makes also the resulting Chromaprint binary GPL licensed.

KissFFT is the slowest option, but it's distributed with a permissive license and
it's very easy to build on platforms like Android.

If you run simple `cmake .`, it will try to find the best library.
If you have FFmpeg installed in a non-standard location,
you can let CMake know using the `FFMPEG_ROOT` option:

    $ cmake -DFFMPEG_ROOT=/path/to/local/ffmpeg/install .

If you are on macOS, Chromaprint will use standard Accelerate framework
with the vDSP library by default. It's very fast and requires you to install
no external libraries.

API Documentation
-----------------

You can use Doxygen to generate a HTML version of the API documentation:

    $ make docs
    $ $BROWSER docs/html/index.html

Unit Tests
----------

The test suite can be built and run using the following commands:

    $ cmake -DBUILD_TESTS=ON .
    $ make check

In order to build the test suite, you will need the sources of the [Google Test][gtest] library.

[gtest]: https://github.com/google/googletest

Related Projects
----------------

Bindings, wrappers and reimplementations in other languages:

 * [Python](https://github.com/beetbox/pyacoustid)
 * [Rust](https://github.com/jameshurst/rust-chromaprint)
 * [Ruby](https://github.com/TMXCredit/chromaprint)
 * [Perl](https://github.com/jonathanstowe/Audio-Fingerprint-Chromaprint)
 * [JavaScript](https://github.com/parshap/node-fpcalc)
 * [JavaScript](https://github.com/bjjb/chromaprint.js) (standalone reimplementation)
 * [Go](https://github.com/go-fingerprint/gochroma)
 * [C#](https://github.com/wo80/AcoustID.NET) (standalone reimplementation)
 * [Pascal](https://github.com/CMCHTPC/ChromaPrint) (standalone reimplementation)
 * [C++/CLI](https://github.com/CyberSinh/Luminescence.Audio)

Integrations:

 * [FFmpeg](https://www.ffmpeg.org/ffmpeg-formats.html#chromaprint-1)
 * [GStreamer](http://cgit.freedesktop.org/gstreamer/gst-plugins-bad/tree/ext/chromaprint)

If you know about a project that is not listed here, but should be, please let me know.

Standing on the Shoulder of Giants
----------------------------------

I've learned a lot while working on this project, which would not be possible
without having information from past research. I've read many papers, but the
concrete ideas implemented in this library are based on the following papers:

 * Yan Ke, Derek Hoiem, Rahul Sukthankar. Computer Vision for Music
   Identification, Proceedings of Computer Vision and Pattern Recognition, 2005.
   http://www.cs.cmu.edu/~yke/musicretrieval/

 * Frank Kurth, Meinard Müller. Efficient Index-Based Audio Matching, 2008.
   http://dx.doi.org/10.1109/TASL.2007.911552

 * Dalwon Jang, Chang D. Yoo, Sunil Lee, Sungwoong Kim, Ton Kalker.
   Pairwise Boosted Audio Fingerprint, 2009.
   http://dx.doi.org/10.1109/TIFS.2009.2034452

