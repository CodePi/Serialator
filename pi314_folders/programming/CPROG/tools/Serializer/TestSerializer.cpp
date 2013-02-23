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
	pair<int,float> pr;
	map<string,int> mp;
	Nested n;
	set<int> s;
	list<int> l;
	deque<int> d;
	vector<Nested> vn;	
	enum E { A,B,C } e;

protected:
	void archive(Archive& ar, int version){
		ar & a & b & c & v & str & pr & mp & n & s & l & d & vn ;
		ar & (int&)e; // need to cast enum to int&
	}
};

bool operator==(const Nested&a, const Nested& b){
	return a.x==b.x && a.y==b.y && a.z==b.z;
}

bool operator==(const MyClass&a, const MyClass& b){
	return a.a==b.a && a.b==b.b && a.c==b.c 
		&& a.v==b.v && a.str==b.str && a.pr==b.pr 
		&& a.mp==b.mp && a.n==b.n
		&& a.s==b.s && a.l==b.l && a.d==b.d && a.vn==b.vn
		&& a.e==a.e;
}

int main(){

	// populate object
	MyClass mc;
	mc.a = 13;
	mc.b = 10;
	mc.v.push_back(11);
	mc.v.push_back(12);
	mc.v.push_back(13);
	mc.pr.first = 10;
	mc.pr.second = 10.1f;
	mc.mp["abc"] = 1;
	mc.mp["def"] = 2;
	mc.n.x = 1;
	mc.str = "hello";
	mc.s.insert(10);
	mc.s.insert(11);
	mc.s.insert(12);
	mc.l.push_back(21);
	mc.l.push_back(22);
	mc.l.push_back(23);
	mc.d.push_back(31);
	mc.d.push_back(32);
	mc.d.push_back(33);
	mc.vn.resize(2);
	mc.vn[0].x = 91;
	mc.vn[0].y = 92;
	mc.vn[0].z = 93;
	mc.vn[1].x = 94;
	mc.vn[1].y = 95;
	mc.vn[1].z = 96;
	mc.e = MyClass::C;

	try{

		// test binary stream serialization
		MyClass mc2;
		stringstream ssb;
		mc.binSerialize(ssb);
		mc2.binDeserialize(ssb);
		if(!(mc==mc2)) throw runtime_error("mc2 not equal");
		else cout << "Test mc2 passed\n";
		streampos serialBinSize = ssb.tellp();

		// test binary file serialization
		MyClass mc3;
		mc.binSerializeFile("test.bin");
		mc3.binDeserializeFile("test.bin");
		if(!(mc==mc3)) throw runtime_error("mc3 not equal");
		else cout << "Test mc3 passed\n";
	
		// test binary vector serialization
		MyClass mc4;
		vector<char>buffBin;
		mc.binSerialize(buffBin);
		mc4.binDeserialize(buffBin);
		if(!(mc==mc4)) cerr << "mc4 not equal\n";
		else cout << "Test mc4a passed\n";
		if(buffBin.size()!=serialBinSize) cerr << "buffBin.size() should match serialBinSize\n";
		else cout << "Test mc4b passed\n";

		// test binary char* serialization
		MyClass mc5;
		int maxSizeBin = 10000;
		char* buffBin2 = new char[maxSizeBin];
		int sizeBin = mc.binSerialize(buffBin2,maxSizeBin);
		mc5.binDeserialize(buffBin2,maxSizeBin);
		delete[]buffBin2;
		if(!(mc==mc5)) cerr << "mc5 not equal\n";
		else cout << "Test mc5 passed\n";
		if(sizeBin!=serialBinSize) cerr << "sizeBin should match serialBinSize\n";
		else cout << "Test mc5 passed\n";

		// test text stream serialization
		stringstream sst;
		MyClass mct2;
		mc.textSerialize(sst);
		string serialString = sst.str();
		mct2.textDeserialize(sst);
		if(!(mc==mct2)) cerr << "mct2 not equal\n";
		else cout << "Test mct2 passed\n";
		streampos serialTextSize = sst.tellp();
	
		// test text file serialization
		MyClass mct3;
		mc.textSerializeFile("test.txt");
		mct3.textDeserializeFile("test.txt");
		if(!(mc==mct3)) cerr << "mct3 not equal\n";
		else cout << "Test mct3 passed\n";
	
		// test text vector serialization
		MyClass mct4;
		vector<char>buff;
		mc.textSerialize(buff);
		mct4.textDeserialize(buff);
		if(!(mc==mct4)) cerr << "mct4 not equal\n";
		else cout << "Test mct4a passed\n";
		if(buff.size()!=serialTextSize) cerr << "buff.size() should match serialTextSize\n";
		else cout << "Test mct4b passed\n";
		
		// test text char* serialization
		MyClass mct5;
		int maxSize = 10000;
		char* buff2 = new char[maxSize];
		int size = mc.textSerialize(buff2,maxSize);
		mct5.textDeserialize(buff2,maxSize);
		delete[]buff2;
		if(!(mc==mct5)) cerr << "mct5 not equal\n";
		else cout << "Test mct5a passed\n";
		if(size!=serialTextSize) cerr << "size should match serialTextSize\n";
		else cout << "Test mct5b passed\n";


		cout << serialString << endl;
	}catch(exception&e){
		cerr<<e.what()<<endl;
	}

#ifdef _WIN32
	cout << "Press enter to exit.\n";
	getchar();
#endif

}
