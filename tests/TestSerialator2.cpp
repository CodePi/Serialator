#include <string>
#include <vector>
#include <map>
#include <sstream>

#include "Serialator.h"

using namespace std;
using namespace codepi;

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

  // Serialize to binary buffer then deseerialize
  Struct1 st1c;
  vector<char> buffer;
  st1.binSerialize(buffer);
  st1c.binDeserialize(buffer);

  // Serialize to binary file then deserialize
  Struct1 st1d;
  st1.binSerializeFile("test2.bin");
  st1c.binDeserializeFile("test2.bin");

  // Compare resulting structures
  st1.textSerialize(cout);
  cout << endl;
  st1b.textSerialize(cout);
  cout << endl;
  st1c.textSerialize(cout);
  cout << endl;
}
