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

// TODO:
// - use catch and rethown to build stack dump on exception

#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "type_traits_helper.h"

#if defined(_MSC_VER) && _MSC_VER < 1600 // if Visual Studio before 2010
typedef int int32_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

namespace Serialator{

class Serializer;  // Forward declaration

///////////////////////////////////////////////////////////////////////////////////////////
// Archive Class
//   Helper class for Serializer
//   This class shouldn't be instantiated directly, only used within Serializer::archive method 
class Archive{
public:
	enum ArchiveType{INIT,READ_BIN,WRITE_BIN,READ_TEXT,WRITE_TEXT,SERIAL_SIZE_BIN};

	// Constructors
	Archive(ArchiveType type)                         
		: mType(type), mpIStream(NULL), mpOStream(NULL), mSerializedSize(0){}
	Archive(ArchiveType type, std::istream& istream) 
		: mType(type), mpIStream(&istream), mpOStream(NULL), mSerializedSize(0){}
	Archive(ArchiveType type, std::ostream& ostream) 
		: mType(type), mpIStream(NULL), mpOStream(&ostream), mSerializedSize(0){}
	
	// operator& for serializing and deserializing strings
	Archive& operator& (std::string& var);

	// operator& for serializing and deserializing descendants of Serializer
	Archive& operator& (Serializer& ser);

	// operator& for serializing and deserializing vectors of any supported type
	template <typename T>
	Archive& operator& (std::vector<T>& vec){

		if(mType==INIT){
			vec.clear();
		}else{
			uint32_t size;
			size = vec.size();  // gets vector size (only meaningful if writing)
			(*this) & size;     // reads or writes vector size
			vec.resize(size);   // resizes vector to fit (only meaningful if reading)
			// TODO: bulk read/write if possible
			for(int i=0;i<size;i++){              // read or write each element to stream
				(*this) & vec[i];
			}
		}
		return *this;
	}

	// operator& for serializing and deserializing basic types (int, float, etc...).
	// The enable_if is to prevent template from matching descendants of Serializer
	template <typename T>
	typename TTHelper::enable_if<!TTHelper::is_serializer<T>::value,Archive&>::type
		operator& (T& var){
			switch(mType){
			case INIT:
				var = T();
				break;
			case READ_BIN: 
				mpIStream->read((char*)&var, sizeof(var));
				if(mpIStream->fail()) throw std::runtime_error("READ_BIN: \"other\" read error");
				break;

			case WRITE_BIN: 
				mpOStream->write((char*)&var, sizeof(var));
				if(mpOStream->fail()) throw std::runtime_error("WRITE_BIN: \"other\" write error");
				break;

			case READ_TEXT:  
				*mpIStream >> var;
				mpIStream->ignore(); // skip past space
				if(mpIStream->fail()) throw std::runtime_error("READ_TEXT: \"other\" read error");
				break;

			case WRITE_TEXT:  
				*mpOStream << var << " ";
				break;

			case SERIAL_SIZE_BIN:
				mSerializedSize += sizeof(var);
				break;

			};
			return *this;
	}

protected:
	// Pointer to input stream for deserialization (null otherwise)
	std::istream* mpIStream;
	// Pointer to output stream for serialization (null otherwise)
	std::ostream* mpOStream;
	// Archive type (see enumeration above)
	ArchiveType mType;  
	// Size of serialized data (used by SERIAL_SIZE_BIN&SERIAL_SIZE_TEXT)
	int mSerializedSize;
	// friend
	friend class Serializer;

};

///////////////////////////////////////////////////////////////////////////////////////////
// Serializer class
//   Descendants of this class should implement archive method which is called by all 
//   of the public methods
class Serializer{
public:
	void initAll();                         // init all elements to type default
	
	// Serialize/deserialize to/from stream
	void textSerialize(std::ostream&os);    // serialize to text stream 
	void textDeserialize(std::istream&is);  // deserialize from text stream
	void binSerialize(std::ostream&os);     // serialize to binary stream
	void binDeserialize(std::istream&is);   // deserialize from binary stream

	// Serialize/deserialize to/from char* (deserialize returns size used)
	int  textSerialize(char* blob, int maxBlobSize);   
	void textDeserialize(const char* blob, int blobSize); 
	int  binSerialize(char* blob, int maxBlobSize);    
	void binDeserialize(const char* blob, int blobSize);  

	// Serialize/deserialize to/from vector<char>
	void textSerialize(std::vector<char>& blob);   
	void textDeserialize(const std::vector<char>& blob); 
	void binSerialize(std::vector<char>& blob);    
	void binDeserialize(const std::vector<char>& blob);  

	// Serialize/deserialize to/from file
	void textSerializeFile(const std::string& filename);   
	void textDeserializeFile(const std::string& filename); 
	void binSerializeFile(const std::string& filename);    
	void binDeserializeFile(const std::string& filename);  

	// Version number allowing for backward compatibility.
	// Override this method to change version number.
	// This number is automatically written and read from stream
	//   and gets passed into archive method.
	virtual int32_t getStructVersion() { return 0; } // default version is 0

protected:
	
	// This method should be implemented in descendant class
	virtual void archive(Archive& ar, int32_t structVersion) =0;

	// Allow Archive to access protected methods
	friend class Archive;

	// Allows enable_if in Archive class to work properly
	typedef void is_serializer;

};

}; //namespace Serialator
