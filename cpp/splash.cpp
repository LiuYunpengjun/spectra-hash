#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <openssl/sha.h>

using namespace std;


// Version
const char SPLASH_VERSION = '0';

// Debug mode
const bool DEBUG = false;

// Precision of floating point operations and representations
const double PRECISION = 6;
const long long PRECISION_FACTOR = static_cast<long long>(pow(10, PRECISION));
const double EPS = 1.0e-6;

// Value to scale relative spectra
const double RELATIVE_INTENSITY_SCALE = 1000.0;

// Separator for building spectrum strings
const char ION_SEPARATOR = ' ';

// Full spectrum hash properties
const char ION_PAIR_SEPARATOR = ':';
const int MAX_HASH_CHARATERS_ENCODED_SPECTRUM = 20;

// Histogram properties
const int BINS = 10;
const int BIN_SIZE = 100;

const char INTENSITY_MAP[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c',
    'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'
};
const int FINAL_SCALE_FACTOR = 35;



string sha256(const string s) {
	unsigned char digest[SHA256_DIGEST_LENGTH];

	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, s.c_str(), s.size());
	SHA256_Final(digest, &sha256);

	stringstream ss;

	for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
		ss << hex << setw(2) << setfill('0') << (int)digest[i];
	}

	return ss.str();
}


// http://stackoverflow.com/a/236803
vector<string> split(const string &s, char delimeter) {
	vector<string> elements;
	stringstream ss(s);
	string element;

	while(getline(ss, element, delimeter)) {
		elements.push_back(element);
	}

	return elements;
}


bool ionPairMzComparator(const pair<double, double> a, const pair<double, double> b) {
	if(abs(a.first - b.first) < EPS) {
		return a.second > b.second;
	} else {
		return a.first < b.first;
	}
}


bool ionPairIntensityComparator(const pair<double, double> a, const pair<double, double> b) {
	if(abs(a.second - b.second) < EPS) {
		return a.first < b.first;
	} else {
		return a.second > b.second;
	}
}


string buildInitialBlock(vector<pair<double, double> > &spectrum, char spectrum_type) {
	stringstream ss;
	ss << "splash" << spectrum_type << SPLASH_VERSION;
	return ss.str();
}

string encodeSpectrum(vector<pair<double, double> > &spectrum, char spectrum_type) {
	sort(spectrum.begin(), spectrum.end(), ionPairMzComparator);

	int i = 0;
	stringstream ss;

	for(vector<pair<double, double> >::iterator it = spectrum.begin(); it != spectrum.end(); ++it) {
		ss << static_cast<long long>((*it).first * PRECISION_FACTOR) << ION_PAIR_SEPARATOR
		   << static_cast<long long>((*it).second * PRECISION_FACTOR);

		if(++i < spectrum.size()) {
			ss << ION_SEPARATOR;
		}
	}

	if(DEBUG) {
		cerr << "Encoded Spectrum: '" << ss.str() << "'" << endl;
	}

	return sha256(ss.str()).substr(0, MAX_HASH_CHARATERS_ENCODED_SPECTRUM);
}


string calculateHistogram(vector<pair<double, double> > &spectrum, char spectrum_type) {
	vector<double> histogram;

	// Bin ions
	for(vector<pair<double, double> >::iterator it = spectrum.begin(); it != spectrum.end(); ++it) {
		int idx = static_cast<int>((*it).first / BIN_SIZE);

		while(histogram.size() <= idx) {
			histogram.push_back(0.0);
		}

		histogram[idx] += (*it).second;
	}

	// Ensure that the histogram has at least BINS bins
	while(histogram.size() < BINS) {
		histogram.push_back(0.0);
	}

	// Wrap the histogram
	for(int i = BINS; i < histogram.size(); i++) {
		histogram[i % BINS] += histogram[i];
	}

	// Normalize the histogram and convert to base 36
	double max_intensity = 0;

	for(int i = 0; i < BINS; i++) {
		if(histogram[i] > max_intensity)
			max_intensity = histogram[i];
	}

	stringstream ss;

	for(int i = 0; i < BINS; i++) {
		int bin = static_cast<long>(FINAL_SCALE_FACTOR * histogram[i] / max_intensity);
		ss << INTENSITY_MAP[bin];
	}

    // Return histogram
    return ss.str();
}


string splashIt(vector<pair<double, double> > &spectrum, char spectrum_type) {
	stringstream ss;
	
	ss << buildInitialBlock(spectrum, spectrum_type) << '-';
	ss << calculateHistogram(spectrum, spectrum_type) << '-';
	ss << encodeSpectrum(spectrum, spectrum_type);

	return ss.str();
}


int main(int argc, char** argv) {
	int i = 0;
	string input;
	
	// Log times
	clock_t start = clock();
	double processingTime;
	

	while(getline(cin, input)) {
		++i;
		
		if(DEBUG) {
			cerr << "Spectrum #" << i << endl;
		}

		// Handle input of the form [id],[spectrum string]
		int delim_pos = input.find(',');
		
		string id = input.substr(0, delim_pos);
		string spectrum_string = input.substr(delim_pos + 1);

		// Convert spectrum to a vector of ion pairs and find the max intensity
		vector<string> ion_strings = split(spectrum_string, ' ');
		vector<pair<double, double> > spectrum;
		double maxIntensity = 0;

		for(vector<string>::iterator it = ion_strings.begin(); it != ion_strings.end(); ++it) {
			delim_pos = (*it).find(':');
			double mz = atof((*it).substr(0, delim_pos).c_str());
			double intensity = atof((*it).substr(delim_pos + 1).c_str());

			if(intensity > maxIntensity)
				maxIntensity = intensity;

			// Store ion as a pair object, with 'first' corresponding to m/z
			// and 'second' to intensity
			spectrum.push_back(make_pair(mz, intensity));
		}

		// Normalize spectrum
		for(vector<pair<double, double> >::iterator it = spectrum.begin(); it != spectrum.end(); ++it) {
			(*it).second = (*it).second / maxIntensity * RELATIVE_INTENSITY_SCALE;
		}
		
		// Provide output for large files
		if(i % 10000 == 0) {
			cerr << "processed " << i << " spectra, " << setprecision(2) << fixed
				 << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) / i
				 << " ms average time to splash a spectrum." << endl;
		}

		// Print the spectrum id with the calculated splash id
		cout << splashIt(spectrum, '1') << "," << id << "," << spectrum_string << endl;
	}

	cerr << "finished processing, processing took: " << setprecision(2) << fixed
		 << (std::clock() - start) / (double)CLOCKS_PER_SEC << " s" << endl;
	cerr << "processed " << i << " spectra" << endl;
	cerr << "average time including io to splash a spectrum is: " << setprecision(2) << fixed
		 << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) / i  << " ms" << endl;
}
