#!/usr/bin/env python

import sys
import urllib2
import urllib
import gzip
from cStringIO import StringIO

USER_API_KEY = ''
CLIENT_API_KEY = ''
API_URL = 'http://api.acoustid.org/submit'
#API_URL = 'http://127.0.0.1:8080/submit'
BATCH_SIZE = 50


def read_log_file(input):
    group = {}
    for line in input:
        line = line.strip()
        if not line:
            if group:
                yield group
            group = {}
            continue
        name, value = line.split('=', 1)
        group[name] = value
    if group:
        yield group


def encode_params(data):
    encoded_body = StringIO()
    encoded_file = gzip.GzipFile(mode='w', fileobj=encoded_body, compresslevel=9)
    encoded_file.write(urllib.urlencode(data))
    encoded_file.close()
    return encoded_body.getvalue()


def submit_data(entries):
    if not entries:
        return True
    params = { 'user': USER_API_KEY, 'client': CLIENT_API_KEY }
    print 'Submitting...'
    for i, entry in enumerate(e for e in entries if e['LENGTH'] >= 40 and len(e['FINGERPRINT'])>100):
        print '  ', entry['MBID'], entry['FINGERPRINT'][:20] + '...'
        params['mbid.%d' % i] = entry['MBID']
        params['fingerprint.%d' % i] = entry['FINGERPRINT']
        params['length.%d' % i] = entry['LENGTH']
        if 'BITRATE' in entry:
            params['bitrate.%d' % i] = entry['BITRATE']
        if 'FORMAT' in entry:
            params['format.%d' % i] = entry['FORMAT']
    data = encode_params(params)
    request = urllib2.Request(API_URL, data, headers={'Content-Encoding': 'gzip'})
    try:
        urllib2.urlopen(request)
    except urllib2.HTTPError, e:
        print e
        for line in e.readlines():
            print line.rstrip()
        return False
    print 'OK'
    return True


batch = []
for entry in read_log_file(open(sys.argv[1]) if sys.argv[1] != '-' else sys.stdin):
    batch.append(entry)
    if len(batch) >= BATCH_SIZE:
        if not submit_data(batch):
            raise SystemExit(1)
        batch = []
if not submit_data(batch):
    raise SystemExit(1)

