#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
//#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <fileref.h>
#include <tag.h>
#include "fingerprinter.h"
#include "ext/ffmpeg_decoder.h"

using namespace std;

typedef vector<string> string_vector;

void FindFiles(const string &dirname, string_vector *result)
{
	DIR *dirp = opendir(dirname.c_str());
	while (dirp) {
		struct dirent *dp;
		if ((dp = readdir(dirp)) != NULL) {
			struct stat sp;
			string filename = dirname + '/' + string(dp->d_name); 
			stat(filename.c_str(), &sp);
			if (S_ISREG(sp.st_mode)) {
				result->push_back(filename);
			}
			if (S_ISDIR(sp.st_mode) && dp->d_name[0] != '.') {
				FindFiles(filename, result);
			}
		}
		else {
			break;
		}
    }
    closedir(dirp);
}

string_vector FindFiles(const char *dirname)
{
	string_vector result;
	FindFiles(dirname, &result);
	sort(result.begin(), result.end());
	return result;
}

bool ReadTags(const string &filename)
{
	TagLib::FileRef file(filename.c_str(), true);
	if (file.isNull())
		return false;
	TagLib::Tag *tags = file.tag();	
	TagLib::AudioProperties *props = file.audioProperties();
	if (!tags || !props)
		return false;
	cout << "ARTIST=" << tags->artist().to8Bit(true) << "\n";
	cout << "TITLE=" << tags->title().to8Bit(true) << "\n";
	cout << "ALBUM=" << tags->album().to8Bit(true) << "\n";
	cout << "LENGTH=" << props->length() << "\n";
	cout << "BITRATE=" << props->bitrate() << "\n";
	return true;
}

string ExtractExtension(const string &filename)
{
	size_t pos = filename.find_last_of('.');
	if (pos == string::npos) {
		return string();
	}
	return boost::to_upper_copy(filename.substr(pos + 1));
}

string EncodeFingerprint(const vector<uint32_t> &fp)
{
	string res;
	res.resize(fp.size());
}

bool ProcessFile(Chromaprint::Fingerprinter *fingerprinter, const string &filename)
{
	Decoder decoder(filename);
	if (!decoder.Open())
		return false;
	if (!ReadTags(filename))
		return false;
	cout << "FILENAME=" << filename << "\n";
	cout << "FORMAT=" << ExtractExtension(filename) << "\n";
	fingerprinter->Init(decoder.SampleRate(), decoder.Channels());
	decoder.Decode(fingerprinter, 60);
	vector<int32_t> fp = fingerprinter->Calculate();
	cout << "FINGERPRINT=";
	cout.setf(ios_base::hex, ios::basefield);
	for (vector<int32_t>::const_iterator it = fp.begin(); it != fp.end(); ++it) {
		cout << *it << " ";
	}
	cout.unsetf(ios_base::hex);
	cout << "\n\n";
	return true;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " DIR\n";
		return 1;
	}

	Chromaprint::Fingerprinter fingerprinter;
	string_vector files = FindFiles(argv[1]);
	for (string_vector::iterator it = files.begin(); it != files.end(); it++) {
		ProcessFile(&fingerprinter, *it);
	}

	return 0;
}

