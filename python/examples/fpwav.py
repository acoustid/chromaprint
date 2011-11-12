#!/usr/bin/env python

# Copyright (C) 2011 Lukas Lalinsky
# Distributed under the MIT license, see the LICENSE file for details. 

# This script just demonstrates how to use the API. See pyacoustid for a
# complete Chromaprint/Acoustid library written in Python:
# https://github.com/sampsyo/pyacoustid

import wave
import chromaprint
from optparse import OptionParser

parser = OptionParser(usage="usage: %prog [options] filename")
parser.add_option("-l", "--length", dest="length", default=30, type='int',
                  help="fingerprint audio length", metavar="LENGTH")
(options, args) = parser.parse_args()

if not args:
    parser.error("missing filename")

wav = wave.open(args[0], 'r')

fpgen = chromaprint.Fingerprinter()
fpgen.start(wav.getframerate(), wav.getnchannels())
length = 0
while length < options.length:
    frames = wav.readframes(1024 * 4)
    if not frames:
        break
    fpgen.feed(frames)
    length += float(len(frames)) / wav.getsampwidth() / wav.getnchannels() / wav.getframerate()
fp = fpgen.finish()

total_length = wav.getnframes() / wav.getframerate()
print "FILE:", args[0]
print "CHANNELS:", wav.getnchannels()
print "SAMPLE RATE:", wav.getframerate()
print "LENGTH:", total_length
print "FINGERPRINT:", ', '.join(map(str, chromaprint.decode_fingerprint(fp)[0]))
print "ENCODED FINGERPRINT:", fp

url = "http://api.acoustid.org/lookup?client=8XaBELgH&meta=1&length=%s&fingerprint=%s" % (total_length, fp)
print
print url

