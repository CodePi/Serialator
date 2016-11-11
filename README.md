# Serialator
Boost-serialize inspired stand alone binary serializer for hierarchical C++ structs

Compiles with C++11 compatible compilers. Tested with gcc 4.4 and Visual Studio 2012.

#### Example usage
```cpp
class Struct1 : public Serialator{

public:
  int exampleIntValue;
  float exampleFloat;
  string exampleString;
  vector<int> exampleVector;
  map<string, int> exampleMap;

protected:
  // This archive method is used by all serialize/deserialize methods
  void archive(Archive& ar, int version){
    ar & exampleIntValue & exampleFloat & exampleString
      & exampleVector & exampleMap;
  }

};

int main(){

  // populate initial structure
  Struct1 origStruct;
  origStruct.exampleIntValue = 1;
  origStruct.exampleString = "an example string";
  origStruct.exampleVector = { 1, 2, 3 };

  // Serialize to binary stream then deserialize
  stringstream ss;
  origStruct.binSerialize(ss);
  Struct1 structFromStream;
  structFromStream.binDeserialize(ss);

  // Serialize to binary buffer then deserialize
  vector<char> buffer;
  origStruct.binSerialize(buffer);
  Struct1 structFromVec;
  structFromVec.binDeserialize(buffer);

  // Serialize to binary file then deserialize
  origStruct.binSerializeFile("test2.bin");
  Struct1 structFromFile;
  structFromFile.binDeserializeFile("test2.bin");

  // Compare resulting structures
  origStruct.textSerialize(cout);
  cout << endl;
  structFromStream.textSerialize(cout);
  cout << endl;
  structFromVec.textSerialize(cout);
  cout << endl;
  structFromFile.textSerialize(cout);
  cout << endl;
}
```

#### Useful Serialator methods
``` cpp
class Serialator{
public:
  void initAll();                         // init all elements to type default

  // Serialize/deserialize to/from stream
  void textSerialize  (std::ostream&os);  // serialize to text stream
  void textDeserialize(std::istream&is);  // deserialize from text stream
  void binSerialize   (std::ostream&os);  // serialize to binary stream
  void binDeserialize (std::istream&is);  // deserialize from binary stream

  // Serialize/deserialize to/from char* (deserialize returns size used)
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
};
