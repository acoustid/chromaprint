# Copyright (C) 2011 Lukas Lalinsky
# Distributed under the MIT license, see the LICENSE file for details. 

import sys
import os
import ctypes


def _guess_lib_name():
    if sys.platform == 'darwin':
        return ('libchromaprint.0.dylib',)
    elif sys.platform == 'win32':
        return ('chromaprint.dll', 'libchromaprint.dll')
    return ('libchromaprint.so.0',)


for name in _guess_lib_name():
    _libchromaprint = ctypes.cdll.LoadLibrary(name)
    break
else:
    raise ImportError("couldn't find libchromaprint")


_libchromaprint.chromaprint_get_version.argtypes = ()
_libchromaprint.chromaprint_get_version.restype = ctypes.c_char_p

_libchromaprint.chromaprint_new.argtypes = (ctypes.c_int,)
_libchromaprint.chromaprint_new.restype = ctypes.c_void_p

_libchromaprint.chromaprint_free.argtypes = (ctypes.c_void_p,)
_libchromaprint.chromaprint_free.restype = None

_libchromaprint.chromaprint_start.argtypes = (ctypes.c_void_p, ctypes.c_int, ctypes.c_int)
_libchromaprint.chromaprint_start.restype = ctypes.c_int

_libchromaprint.chromaprint_feed.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_char), ctypes.c_int)
_libchromaprint.chromaprint_feed.restype = ctypes.c_int

_libchromaprint.chromaprint_finish.argtypes = (ctypes.c_void_p,)
_libchromaprint.chromaprint_finish.restype = ctypes.c_int

_libchromaprint.chromaprint_get_fingerprint.argtypes = (ctypes.c_void_p, ctypes.POINTER(ctypes.c_char_p))
_libchromaprint.chromaprint_get_fingerprint.restype = ctypes.c_int

_libchromaprint.chromaprint_decode_fingerprint.argtypes = (ctypes.POINTER(ctypes.c_char), ctypes.c_int, ctypes.POINTER(ctypes.POINTER(ctypes.c_int32)), ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int), ctypes.c_int)
_libchromaprint.chromaprint_decode_fingerprint.restype = ctypes.c_int

_libchromaprint.chromaprint_encode_fingerprint.argtypes = (ctypes.POINTER(ctypes.c_int32), ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.POINTER(ctypes.c_char)), ctypes.POINTER(ctypes.c_int), ctypes.c_int)
_libchromaprint.chromaprint_encode_fingerprint.restype = ctypes.c_int

_libchromaprint.chromaprint_dealloc.argtypes = (ctypes.c_void_p,)
_libchromaprint.chromaprint_dealloc.restype = None


class Fingerprinter(object):

    ALGORITHM_TEST1 = 0
    ALGORITHM_TEST2 = 1
    ALGORITHM_TEST3 = 2
    ALGORITHM_DEFAULT = ALGORITHM_TEST2

    def __init__(self, algorithm=ALGORITHM_DEFAULT):
        self._ctx = _libchromaprint.chromaprint_new(algorithm)

    def __del__(self):
        _libchromaprint.chromaprint_free(self._ctx)
        del self._ctx

    def start(self, sample_rate, num_channels):
        _libchromaprint.chromaprint_start(self._ctx, sample_rate, num_channels)

    def feed(self, data):
        _libchromaprint.chromaprint_feed(self._ctx, data, len(data) / 2)

    def finish(self):
        _libchromaprint.chromaprint_finish(self._ctx)
        fingerprint_ptr = ctypes.c_char_p()
        _libchromaprint.chromaprint_get_fingerprint(self._ctx, ctypes.byref(fingerprint_ptr))
        fingerprint = fingerprint_ptr.value
        _libchromaprint.chromaprint_dealloc(fingerprint_ptr)
        return fingerprint

def decode_fingerprint(data, base64=True):
    result_ptr = ctypes.POINTER(ctypes.c_int32)()
    result_size = ctypes.c_int()
    algorithm = ctypes.c_int()
    _libchromaprint.chromaprint_decode_fingerprint(data, len(data), ctypes.byref(result_ptr), ctypes.byref(result_size), ctypes.byref(algorithm), 1 if base64 else 0)
    result = result_ptr[:result_size.value]
    _libchromaprint.chromaprint_dealloc(result_ptr)
    return result, algorithm.value

def encode_fingerprint(decoded_fingerprint, base64=True):
	fp_data, algorithm = decoded_fingerprint
	fp_array = (ctypes.c_int * len(fp_data))()
	for i in range(len(fp_data)):
		fp_array[i] = fp_data[i]
	result_ptr = ctypes.POINTER(ctypes.c_char)()
	result_size = ctypes.c_int()
	_libchromaprint.chromaprint_encode_fingerprint(fp_array, len(fp_data), algorithm, ctypes.byref(result_ptr), ctypes.byref(result_size), 1 if base64 else 0)
	result = result_ptr[:result_size.value]
	_libchromaprint.chromaprint_dealloc(result_ptr)
	return result

