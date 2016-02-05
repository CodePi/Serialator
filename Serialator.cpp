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

#include "Serialator.h"
#include <fstream>
#include <sstream>

namespace codepi{

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

// constructors
Archive::Archive(ArchiveType type)                         
	: mType(type), mpIStream(NULL), mpOStream(NULL), mSerializedSize(0){
		if(type!=INIT && type!=SERIAL_SIZE_BIN){
			throw runtime_error("Init/size Archive constructor is not compatible with type");
		}
}

Archive::Archive(ArchiveType type, istream& istream) 
	: mType(type), mpIStream(&istream), mpOStream(NULL), mSerializedSize(0){
		if(type!=READ_BIN && type!=READ_TEXT){
			throw runtime_error("Read Archive constructor is not compatible with type");
		}
}

Archive::Archive(ArchiveType type, ostream& ostream) 
	: mType(type), mpIStream(NULL), mpOStream(&ostream), mSerializedSize(0){
		if(type!=WRITE_BIN && type!=WRITE_TEXT){
			throw runtime_error("Write Archive constructor is not compatible with type");
		}
}

// operator& implementation for serializing and deserializing strings
Archive& Archive::operator&(string& var){
	uint32_t size;

	switch(mType){
	case INIT:
		var.clear();
		break;

	case READ_BIN: 
		(*this) & size;   // read string size from stream
		var.resize(size); // resize string
		if(size>0) mpIStream->read((char*)var.c_str(), size); // read string from stream
		if(mpIStream->fail()) throw runtime_error("READ_BIN: string read error");
		break; 

	case WRITE_BIN: 
		size = var.size();
		(*this) & size;   // write string size to stream
		if(size>0) mpOStream->write(var.c_str(), size); // write string to stream
		if(mpOStream->fail()) throw runtime_error("WRITE_BIN: string write error");
		break; 

	case READ_TEXT:
		(*this) & size;   // read string size from stream
		var.resize(size); // resize string
		if(size>0) mpIStream->read((char*)var.c_str(), size);  // read string
		mpIStream->ignore(); // skip past space
		if(mpIStream->fail()) throw runtime_error("READ_TEXT: string read error");
		break;

	case WRITE_TEXT:
		size = var.size();
		(*this) & size;   // write string size to stream
		*mpOStream << var << " ";  // output string and space
		if(mpOStream->fail()) throw runtime_error("WRITE_TEXT: string read error");
		break;

	case SERIAL_SIZE_BIN:
		mSerializedSize += sizeof(size) + var.size();
		break;

	default: 
		throw runtime_error("string operator& switch hit default.  Code error"); 
		break;

	};
	return *this;
}

// operator& for serializing and deserializing descendants of Serialator
Archive& Archive::operator& (Serialator& ser){
	int32_t version = ser.getStructVersion();
	if(mType!=INIT) *this & version;  // read or write version number (if not init)
	ser.archive(*this, version);
	return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Serialization method implementations

// init all elements to type default
void Serialator::initAll(){ 
	Archive ar(Archive::INIT);
	ar & *this;
}

//////////////////////////////////////////
// Serialize/deserialize to/from stream //
//////////////////////////////////////////

void Serialator::textSerialize(std::ostream&os){
	Archive ar(Archive::WRITE_TEXT,os);
	ar & *this;
}

void Serialator::textDeserialize(std::istream&is){
	Archive ar(Archive::READ_TEXT,is);
	ar & *this;
}

void Serialator::binSerialize(std::ostream&os){
	Archive ar(Archive::WRITE_BIN,os);
	ar & *this;
}

void Serialator::binDeserialize(std::istream&is){
	Archive ar(Archive::READ_BIN,is);
	ar & *this;
}

/////////////////////////////////////////
// Serialize/deserialize to/from char* //
/////////////////////////////////////////

int Serialator::textSerialize(char* blob, int maxBlobSize){
	StreambufWrapper sb(blob, maxBlobSize); // create streambuf from blob without copying
	ostream os(&sb);                        // wrap streambuf in ostream
	textSerialize(os);                      // serialize to streambuf
	int size = sb.getSizeUsed();            
	if(size<maxBlobSize) blob[size]=0;      // add \0 if there is room
	return size;                            // return used size
}

void Serialator::textDeserialize(const char* blob, int blobSize){
	StreambufWrapper sb((char*)blob, blobSize); // create streambuf from blob without copying
	istream is(&sb);                            // wrap streambuf in istream
	textDeserialize(is);                        // deserialize from streambuf
}

int Serialator::binSerialize(char* blob, int maxBlobSize){
	StreambufWrapper sb(blob, maxBlobSize); // create streambuf from blob without copying
	ostream os(&sb);                        // wrap streambuf in ostream
	binSerialize(os);                       // serialize to streambuf
	return sb.getSizeUsed();                // return used size
}

void Serialator::binDeserialize(const char* blob, int blobSize){
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

void Serialator::textSerialize(vector<char>& blob){
	stringstream ss;                      
	textSerialize(ss);           // serialize to stream
	int size = ss.tellp();       // get size of stream
	blob.resize(size);           // resize blob
	ss.read(vecptr(blob), size);  // copy stream to buffer
}

void Serialator::textDeserialize(const vector<char>& blob){
	textDeserialize(vecptr(blob), blob.size());
}

void Serialator::binSerialize(vector<char>& blob){
	Archive ar(Archive::SERIAL_SIZE_BIN);   // setup size calculating archive
	ar & *this;                             // calculate size
	blob.resize(ar.mSerializedSize);        // resize blob to calculated size
	binSerialize(vecptr(blob), blob.size()); // serialize to blob
}

void Serialator::binDeserialize(const vector<char>& blob){
	binDeserialize(blob.data(), blob.size());
}

//////////////////////////////////////////
// Serialize/deserialize to/from file   //
//////////////////////////////////////////

void Serialator::textSerializeFile(const std::string& filename){
	ofstream ofs(filename.c_str());
	if(ofs.fail()) throw runtime_error("textSerializeFile: cannot open file");
	textSerialize(ofs);
}

void Serialator::textDeserializeFile(const std::string& filename){
	ifstream ifs(filename.c_str());
	if(ifs.fail()) throw runtime_error("textSerializeFile: cannot open file");
	textDeserialize(ifs);
}

void Serialator::binSerializeFile(const std::string& filename){
	ofstream ofs(filename.c_str(), ios::binary);
	if(ofs.fail()) throw runtime_error("binSerializeFile: cannot open file");
	binSerialize(ofs);
}

void Serialator::binDeserializeFile(const std::string& filename){
	ifstream ifs(filename.c_str(), ios::binary);
	if(ifs.fail()) throw runtime_error("binDeserializeFile: cannot open file");
	binDeserialize(ifs);
}


}; //end namespace codepi
