#include <sstream>
#include <fstream>
#include <string>
#include <assert.h>
#include "Serializer.h"

using namespace std;
using namespace Serialator;

class Nested : public Serializer{
public:
	int x,y,z;
protected:
	void archive(Archive& ar, int version){
		ar & x & y & z; 
	}
};

class MyClass : public Serializer{
public:
	MyClass(){initAll();}
	int a,b,c;
	vector<int>v;
	string str;
	pair<int,float> pair;
	map<string,int> mp;
	Nested n;
protected:
	void archive(Archive& ar, int version){
		ar & a & b & c & v & str & pair & mp & n; 
	}
};

bool operator==(Nested&a, Nested& b){
	return a.x==b.x && a.y==b.y && a.z==b.z;
}

bool operator==(MyClass&a, MyClass& b){
	return a.a==b.a && a.b==b.b && a.c==b.c 
		&& a.v==b.v && a.str==b.str && a.pair==b.pair 
		&& a.mp==b.mp && a.n==b.n;
}

int main(){

	// populate object
	MyClass mc;
	mc.a = 13;
	mc.b = 10;
	mc.v.push_back(11);
	mc.v.push_back(12);
	mc.v.push_back(13);
	mc.pair.first = 10;
	mc.pair.second = 10.1;
	mc.mp["abc"] = 1;
	mc.mp["def"] = 2;
	mc.n.x = 1;
	mc.str = "hello";

	try{

		// test binary stream serialization
		MyClass mc2;
		stringstream ssb;
		mc.binSerialize(ssb);
		mc2.binDeserialize(ssb);
		if(!(mc==mc2)) throw runtime_error("mc2 not equal");
		int serialBinSize = ssb.tellp();

		// test binary file serialization
		MyClass mc3;
		mc.binSerializeFile("test.bin");
		mc3.binDeserializeFile("test.bin");
		if(!(mc==mc3)) throw runtime_error("mc3 not equal");
	
		// test binary vector serialization
		MyClass mc4;
		vector<char>buffBin;
		mc.binSerialize(buffBin);
		mc4.binDeserialize(buffBin);
		if(!(mc==mc4)) cerr << "mc4 not equal\n";
		if(buffBin.size()!=serialBinSize) cerr << "buffBin.size() should match serialBinSize\n";

		// test binary char* serialization
		MyClass mc5;
		int maxSizeBin = 10000;
		char* buffBin2 = new char[maxSizeBin];
		int sizeBin = mc.binSerialize(buffBin2,maxSizeBin);
		mc5.binDeserialize(buffBin2,maxSizeBin);
		delete[]buffBin2;
		if(!(mc==mc5)) cerr << "mc5 not equal\n";
		if(sizeBin!=serialBinSize) cerr << "sizeBin should match serialBinSize\n";

		// test text stream serialization
		stringstream sst;
		MyClass mct2;
		mc.textSerialize(sst);
		string serialString = sst.str();
		mct2.textDeserialize(sst);
		if(!(mc==mct2)) cerr << "mct2 not equal\n";
		int serialTextSize = sst.tellp();
	
		// test text file serialization
		MyClass mct3;
		mc.textSerializeFile("test.txt");
		mct3.textDeserializeFile("test.txt");
		if(!(mc==mct3)) cerr << "mct3 not equal\n";
	
		// test text vector serialization
		MyClass mct4;
		vector<char>buff;
		mc.textSerialize(buff);
		mct4.textDeserialize(buff);
		if(!(mc==mct4)) cerr << "mct4 not equal\n";
		if(buff.size()!=serialTextSize) cerr << "buff.size() should match serialTextSize\n";
		
		// test text char* serialization
		MyClass mct5;
		int maxSize = 10000;
		char* buff2 = new char[maxSize];
		int size = mc.textSerialize(buff2,maxSize);
		mct5.textDeserialize(buff2,maxSize);
		delete[]buff2;
		if(!(mc==mct5)) cerr << "mct5 not equal\n";
		if(size!=serialTextSize) cerr << "size should match serialTextSize\n";

	}catch(exception&e){
		cerr<<e.what()<<endl;
	}
}
