// Copyright (C) 2011 Paul Ilardi (http://github.com/CodePi)
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

// Tested with Visual Studio 2010 
// Tested with g++ 4.6 & 4.7 with -std=c++11 flag
// Should work with g++ 4.3 and later with -std=c++0x flag

// TODO:
// - use catch and rethown to build stack dump on exception

#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <stdexcept>
#include <type_traits>

#if defined(_MSC_VER) && _MSC_VER < 1600 // if Visual Studio before 2010
typedef int int32_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

namespace codepi{

class Serialator;  // Forward declaration

///////////////////////////////////////////////////////////////////////////////////////////
// Archive Class
//   Helper class for Serialator
//   This class shouldn't be instantiated directly, only used within Serialator::archive method 
class Archive{
public:
  enum ArchiveType{
    INIT,            // Initialize all elements to type default
    READ_BIN,        // Read from binary stream
    WRITE_BIN,       // Write to binary stream
    READ_TEXT,       // Read from text stream
    WRITE_TEXT,      // Write to text stream
    SERIAL_SIZE_BIN  // Calculate binary serialized size
  };

  // Constructors
  Archive(ArchiveType type);                        // For INIT or SERIAL_SIZE_BIN
  Archive(ArchiveType type, std::istream& istream); // For READ_BIN or READ_TEXT
  Archive(ArchiveType type, std::ostream& ostream); // For WRITE_BIN or WRITE_TEXT
  
  // operator& for serializing and deserializing strings
  Archive& operator& (std::string& var);

  // operator& for serializing and deserializing descendants of Serialator
  Archive& operator& (Serialator& ser);

  // operator& for serializing and deserializing vectors
  template <typename T>
  Archive& operator& (std::vector<T>& vec){
    if(mType==INIT) vec.clear();
    else{
      uint32_t size = vec.size(); // get size (if writing)
      (*this) & size;             // read or write size
      vec.resize(size);           // resize (if reading)

      // binary read of contiguous values
      if(mType==READ_BIN && std::is_arithmetic<T>::value){
        if(size>0) mpIStream->read((char*)vec.data(), sizeof(T)*size); // read all data
        if(mpIStream->fail()) throw std::runtime_error("READ_BIN: \"vector\" read error");

      // binary write of contiguous values
      }else if(mType==WRITE_BIN && std::is_arithmetic<T>::value){
        if(size>0) mpOStream->write((char*)vec.data(), sizeof(T)*size);
        if(mpOStream->fail()) throw std::runtime_error("WRITE_BIN: \"vector\" write error");

      // get binary size of contiguous values
      }else if(mType==SERIAL_SIZE_BIN && std::is_arithmetic<T>::value){
        mSerializedSize += sizeof(T)*size;
        
      // cases not covered by above (text and non-contiguous)
      }else{ // READ_TEXT or WRITE_TEXT
        for(uint32_t i=0;i<size;i++) (*this) & vec[i];
      }
    }

    return *this;
  }

  // operator& for serializing and deserializing std::array
  template <typename T, size_t N>
  Archive& operator& (std::array<T,N>& arr){
    if(mType==INIT) arr.fill(T());
    else{
      uint32_t size = arr.size(); // get size (if writing)
      (*this) & size;             // read or write size
      if(size > arr.size()) throw std::runtime_error("operator& array size error");

      // binary read of contiguous values
      if(mType==READ_BIN && std::is_arithmetic<T>::value){
        if(size>0) mpIStream->read((char*)arr.data(), sizeof(T)*size); // read all data
        if(mpIStream->fail()) throw std::runtime_error("READ_BIN: \"array\" read error");

      // binary write of contiguous values
      }else if(mType==WRITE_BIN && std::is_arithmetic<T>::value){
        if(size>0) mpOStream->write((char*)arr.data(), sizeof(T)*size);
        if(mpOStream->fail()) throw std::runtime_error("WRITE_BIN: \"array\" write error");

      // get binary size of contiguous values
      }else if(mType==SERIAL_SIZE_BIN && std::is_arithmetic<T>::value){
        mSerializedSize += sizeof(T)*size;
        
      // cases not covered by above (text and non-contiguous)
      }else{ 
        for(uint32_t i=0;i<size;i++) (*this) & arr[i];
      }
    }

    return *this;
  }

  // operator& for serializing and deserializing maps of any supported types
  template <typename T1, typename T2>
  Archive& operator& (std::map<T1,T2>& mp){
    containerHelper(mp);
    return *this;
  }

  // operator& for serializing and deserializing sets of any supported types
  template <typename T>
  Archive& operator& (std::set<T>& s){
    containerHelper(s);
    return *this;
  }

  // operator& for serializing and deserializing lists of any supported types
  template <typename T>
  Archive& operator& (std::list<T>& l){
    containerHelper(l);
    return *this;
  }

  // operator& for serializing and deserializing deques of any supported types
  template <typename T>
  Archive& operator& (std::deque<T>& d){
    containerHelper(d);
    return *this;
  }

  // operator& for serializing and deserializing pairs of any supported types
  template <typename T1, typename T2>
  Archive& operator& (std::pair<T1,T2>& pair){
    // Need const castoff to prevent compiler error when using std::map
    // Value won't actually change in the cases where it's const, 
    //   but compiler doesn't realize it.
    (*this) & remove_const(pair.first) & pair.second;
    return *this;
  }

  // operator& for serializing and deserializing basic types (int, float, etc...).
  // The enable_if is to prevent template from matching descendants of Serialator
  template <typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, Archive&>::type
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
        if(mpOStream->fail()) throw std::runtime_error("WRITE_TEXT: \"other\" write error");
        break;

      case SERIAL_SIZE_BIN:
        mSerializedSize += sizeof(var);
        break;

      default: 
        throw std::runtime_error("\"other\" operator& switch hit default.  Code error"); 
        break;

      };
      return *this;
  }

private:

  // Pointer to input stream for deserialization (null otherwise)
  std::istream* mpIStream;
  // Pointer to output stream for serialization (null otherwise)
  std::ostream* mpOStream;
  // Archive type (see enumeration above)
  ArchiveType mType;  
  // Size of serialized data (used by SERIAL_SIZE_BIN)
  int mSerializedSize;
  // friend
  friend class Serialator;
  
  /// helper function for handling maps and sets
  /// workaround: map and set value_type contain const this casts off the const
  template <typename T>
  static T& remove_const(const T& val){
    return const_cast<T&>(val);
  }
  
  template <typename Container>
  void containerHelper(Container& container){
    uint32_t size;

    switch(mType){
    case READ_BIN:
    case READ_TEXT:
      container.clear();
      (*this) & size;
      for(uint32_t i=0; i<size; i++){
        typename Container::value_type val;
        (*this) & val;
        container.insert(container.end(), std::move(val));
      }
      break;

    case WRITE_BIN:
    case WRITE_TEXT:
    case SERIAL_SIZE_BIN:
      size = container.size();
      (*this) & size;
      for(typename Container::iterator i=container.begin(); i!=container.end(); i++){
        // Need const castoff to prevent compiler error.
        // Value won't actually change, but compiler doesn't realize it.        
        (*this) & remove_const(*i);
      }
      break;

    case INIT:
      container.clear();
      break;

    default: 
      throw std::runtime_error("containerHelper switch hit default.  Code error"); 
      break;

    }

  }

};

///////////////////////////////////////////////////////////////////////////////////////////
// Serialator class
//   Descendants of this class should implement archive method which is called by all 
//   of the public methods
class Serialator{
public:
  void initAll();                         // init all elements to type default
  
  // Serialize/deserialize to/from stream
  void textSerialize  (std::ostream&os);  // serialize to text stream 
  void textDeserialize(std::istream&is);  // deserialize from text stream
  void binSerialize   (std::ostream&os);  // serialize to binary stream
  void binDeserialize (std::istream&is);  // deserialize from binary stream

  // Serialize/deserialize to/from char* (serialize returns size used)
  int  textSerialize  (      char* blob, int maxBlobSize);   
  void textDeserialize(const char* blob, int blobSize); 
  int  binSerialize   (      char* blob, int maxBlobSize);    
  void binDeserialize (const char* blob, int blobSize);  

  // Serialize/deserialize to/from vector<char>
  void textSerialize  (      std::vector<char>& blob);   
  void textDeserialize(const std::vector<char>& blob); 
  void binSerialize   (      std::vector<char>& blob);    
  void binDeserialize (const std::vector<char>& blob);  

  // Serialize/deserialize to/from file
  void textSerializeFile  (const std::string& filename);   
  void textDeserializeFile(const std::string& filename); 
  void binSerializeFile   (const std::string& filename);    
  void binDeserializeFile (const std::string& filename);  

  // Version number allowing for backward compatibility.
  // Override this method to change version number.
  // This number is automatically written and read from stream
  //   and gets passed into archive method.
  virtual int32_t getStructVersion() { return 0; } // default version is 0

  //virtualized destructor for proper inheritance
  virtual ~Serialator(){}   

protected:
  
  // This method should be implemented in descendant class
  virtual void archive(Archive& ar, int32_t structVersion) =0;

  // Allow Archive to access protected methods
  friend class Archive;

};

}; //end namespace codepi
