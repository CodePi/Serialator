// Copyright (C) 2011 Paul Ilardi (code@pilardi.com)
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, unconditionally.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.

#include "Serializer.h"
#include <fstream>
#include <sstream>

namespace Serialator{

using namespace std;

////////////////////////////////////////////////////////
// Helper class for wrapping a char* without copying
// This allows use of a c-style array a streambuf
// The streambuf can then be used in a istream or ostream

class StreambufWrapper : public streambuf {
public:
	StreambufWrapper(char* s, std::size_t n) {
		setg(s, s, s + n);  // set up as input buffer
		setp(s, s + n);     // set up as output buffer
	}
	size_t getSizeUsed(){
		return pptr() - pbase();
	}
};

///////////////////////////////////////////////////////////////////////////////////////////
// Archive method implementations

// operator& implementation for serializing and deserializing strings
Archive& Archive::operator&(string& var){
	uint32_t size;

	switch(mType){
	case INIT:
		var.clear();
		break;

	case READ_BIN: 
		mpIStream->read((char*)&size, sizeof(size));   // output size to stream
		if(mpIStream->fail()) throw runtime_error("READ_BIN: string size read error");
		var.resize(size);
		mpIStream->read((char*)var.c_str(), size); // output string to stream
		if(mpIStream->fail()) throw runtime_error("READ_BIN: string read error");
		break; 

	case WRITE_BIN: 
		size = var.size();
		mpOStream->write((char*)&size, sizeof(size));   // output size to stream
		mpOStream->write(var.c_str(), size); // output string to stream
		if(mpOStream->fail()) throw runtime_error("WRITE_BIN: string write error");
		break; 

	case READ_TEXT:
		*mpIStream >> size;  // read string size
		mpIStream->ignore(); // skip past space
		if(mpIStream->fail()) throw std::runtime_error("READ_TEXT: string size read error");
		var.resize(size);
		mpIStream->read(&var[0], size);  // read string
		mpIStream->ignore(); // skip past space
		if(mpIStream->fail()) throw runtime_error("READ_TEXT: string read error");
		break;

	case WRITE_TEXT:
		size = var.size();
		*mpOStream << size << " "; // output string size and space
		if(mpOStream->fail()) throw std::runtime_error("WRITE_TEXT vector: vector size write error");
		*mpOStream << var << " ";  // output string and space
		if(mpOStream->fail()) throw runtime_error("WRITE_TEXT: string read error");
		break;

	case SERIAL_SIZE_BIN:
		mSerializedSize += sizeof(int) + var.size();
		break;

	default: throw runtime_error("string operator& switch hit default.  Code error"); break;

	};
	return *this;
}

// operator& for serializing and deserializing descendants of Serializer
Archive& Archive::operator& (Serializer& ser){
	int32_t version = ser.getStructVersion();
	if(mType!=INIT) *this & version;  // read or write version number (if not init)
	ser.archive(*this, version);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Serialization method implementations

// init all elements to type default
void Serializer::initAll(){ 
	Archive ar(Archive::INIT);
	ar & *this;
}

//////////////////////////////////////////
// Serialize/deserialize to/from stream //
//////////////////////////////////////////

void Serializer::textSerialize(std::ostream&os){
	Archive ar(Archive::WRITE_TEXT,os);
	ar & *this;
}

void Serializer::textDeserialize(std::istream&is){
	Archive ar(Archive::READ_TEXT,is);
	ar & *this;
}

void Serializer::binSerialize(std::ostream&os){
	Archive ar(Archive::WRITE_BIN,os);
	ar & *this;
}

void Serializer::binDeserialize(std::istream&is){
	Archive ar(Archive::READ_BIN,is);
	ar & *this;
}

/////////////////////////////////////////
// Serialize/deserialize to/from char* //
/////////////////////////////////////////

int Serializer::textSerialize(char* blob, int maxBlobSize){
	StreambufWrapper sb(blob, maxBlobSize); // create streambuf from blob without copying
	ostream os(&sb);                        // wrap streambuf in ostream
	textSerialize(os);                      // serialize to streambuf
	int size = sb.getSizeUsed();            
	if(size<maxBlobSize) blob[size]=0;      // add \0 if there is room
	return size;                            // return used size
}

void Serializer::textDeserialize(const char* blob, int blobSize){
	StreambufWrapper sb((char*)blob, blobSize); // create streambuf from blob without copying
	istream is(&sb);                            // wrap streambuf in istream
	textDeserialize(is);                        // deserialize from streambuf
}

int Serializer::binSerialize(char* blob, int maxBlobSize){
	StreambufWrapper sb(blob, maxBlobSize); // create streambuf from blob without copying
	ostream os(&sb);                        // wrap streambuf in ostream
	binSerialize(os);                       // serialize to streambuf
	return sb.getSizeUsed();                // return used size
}

void Serializer::binDeserialize(const char* blob, int blobSize){
	StreambufWrapper sb((char*)blob, blobSize); // create streambuf from blob without copying
	istream is(&sb);                            // wrap streambuf in istream
	binDeserialize(is);                         // deserialize from streambuf
}

////////////////////////////////////////////////
// Serialize/deserialize to/from vector<char> //
////////////////////////////////////////////////

// same as vec.data() but compatible with VS2008
static char* vecptr(vector<char>& vec){
	return vec.empty() ? NULL : &vec[0];
}

// same as vec.data() but compatible with VS2008
static const char* vecptr(const vector<char>& vec){
	return vec.empty() ? NULL : &vec[0];
}

void Serializer::textSerialize(vector<char>& blob){
	stringstream ss;                      
	textSerialize(ss);           // serialize to stream
	int size = ss.tellp();       // get size of stream
	blob.resize(size);           // resize blob
	ss.read(vecptr(blob), size);  // copy stream to buffer
}

void Serializer::textDeserialize(const vector<char>& blob){
	textDeserialize(vecptr(blob), blob.size());
}

void Serializer::binSerialize(vector<char>& blob){
	Archive ar(Archive::SERIAL_SIZE_BIN);   // setup size calculating archive
	ar & *this;                             // calculate size
	blob.resize(ar.mSerializedSize);        // resize blob to calculated size
	binSerialize(vecptr(blob), blob.size()); // serialize to blob
}

void Serializer::binDeserialize(const vector<char>& blob){
	binDeserialize(blob.data(), blob.size());
}

//////////////////////////////////////////
// Serialize/deserialize to/from file   //
//////////////////////////////////////////

void Serializer::textSerializeFile(const std::string& filename){
	ofstream ofs(filename.c_str());
	if(ofs.fail()) throw runtime_error("textSerializeFile: cannot open file");
	textSerialize(ofs);
}

void Serializer::textDeserializeFile(const std::string& filename){
	ifstream ifs(filename.c_str());
	if(ifs.fail()) throw runtime_error("textSerializeFile: cannot open file");
	textDeserialize(ifs);
}

void Serializer::binSerializeFile(const std::string& filename){
	ofstream ofs(filename.c_str(), ios::binary);
	if(ofs.fail()) throw runtime_error("binSerializeFile: cannot open file");
	binSerialize(ofs);
}

void Serializer::binDeserializeFile(const std::string& filename){
	ifstream ifs(filename.c_str(), ios::binary);
	if(ifs.fail()) throw runtime_error("binDeserializeFile: cannot open file");
	binDeserialize(ifs);
}


}; //namespace Serialator
