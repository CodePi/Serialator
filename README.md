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
  Struct1 st1;
  st1.exampleIntValue = 1;
  st1.exampleString = "an example string";
  st1.exampleVector = { 1, 2, 3 };

  // Serialize to binary stream then deserialize
  Struct1 st1b;
  stringstream ss;
  st1.binSerialize(ss);
  st1b.binDeserialize(ss);

  // Serialize to binary buffer then deserialize
  Struct1 st1c;
  vector<char> buffer;
  st1.binSerialize(buffer);
  st1c.binDeserialize(buffer);

  // Serialize to binary file then deserialize
  Struct1 st1d;
  st1.binSerializeFile("test2.bin");
  st1d.binDeserializeFile("test2.bin");

  // Compare resulting structures
  st1.textSerialize(cout);
  cout << endl;
  st1b.textSerialize(cout);
  cout << endl;
  st1c.textSerialize(cout);
  cout << endl;
  st1d.textSerialize(cout);
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
