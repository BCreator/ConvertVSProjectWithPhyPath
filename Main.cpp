#include<conio.h>
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "tinyxml/tinyxml.h"
#include <iostream>
#include <vector>
#include <set>
using namespace std;

#ifdef WIN32
//guid
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif

GUID CreateGuid()
{
	GUID guid;
#ifdef WIN32
	CoCreateGuid(&guid);
#else
	uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#endif
	return guid;
}

std::string GuidToString(const GUID &guid)
{
	char buf[64] = { 0 };
#ifdef __GNUC__
	snprintf(
#else // MSVC
	_snprintf_s(
#endif
		buf,
		sizeof(buf),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return std::string(buf);
}

void subDir(vector<string>& arr, string str, char sp)
{
	arr.push_back(str);
	for (size_t pos = str.find_last_of(sp); pos != string::npos; pos = str.find_last_of(sp))
	{
		str = str.substr(0, pos);
		arr.push_back(str);
	}
}

int main()
{
	set<string> OldDirset;
	set<string> NewDirset;
	char tmp[256];

	cout << "输入文件名！" << endl;
	cin >> tmp;
	string filepath = tmp;
	TiXmlDocument doc(filepath.c_str());

	bool b = doc.LoadFile();
	if (!b) {
		printf("Error: 加载文件失败！！按任意键退出。");//by houstond
		_getch();
		return 0;
	}
	printf("文件加载成功!\n");
	//std::cout<<doc.Value()<<std::endl;
	TiXmlElement* project = doc.FirstChildElement();	//第一个Project元素
	//cout << project->Value() << project->Attribute("xmlns")<<endl;
	//cout << project->FirstChild()->NextSibling()->Value();//获取第二个
	TiXmlElement* FirstItemGroup = project->FirstChildElement();

	//查找已有信息
	{
		TiXmlElement* Filter = FirstItemGroup->FirstChildElement();
		//cout << Filter->Value() << endl;
		for (; Filter != NULL; Filter = Filter->NextSiblingElement())
		{
			string dir = Filter->Attribute("Include");
			OldDirset.insert(dir);
		}
	}
	//查找未添加的
	TiXmlElement* ItemGroup = FirstItemGroup->NextSiblingElement();
//	for (int i = 0; ItemGroup != NULL && i < 2; ItemGroup = ItemGroup->NextSiblingElement(), i++)
	for (;ItemGroup != NULL ; ItemGroup = ItemGroup->NextSiblingElement() )
	{
		TiXmlElement* ClCompile = ItemGroup->FirstChildElement();
		for (; ClCompile != NULL; ClCompile = ClCompile->NextSiblingElement())
		{
			string dir = ClCompile->Attribute("Include");
			TiXmlElement* Filter = ClCompile->FirstChildElement();
			dir = dir.substr(0, dir.find_last_of('\\'));
			if (!Filter) {		//by houstond
				Filter = new TiXmlElement("Filter");
				Filter->LinkEndChild(new TiXmlText("temp"));
				ClCompile->LinkEndChild(Filter);
			}
			Filter->FirstChild()->ToText()->SetValue(dir.c_str());
			//cout << dir.c_str() << endl;
			vector<string> arr;
			subDir(arr, dir, '\\');
			for (int i = 0; i < arr.size(); i++)
			{
				if (OldDirset.insert(arr[i]).second)
				{
					NewDirset.insert(arr[i]);
				}
			}
		}
	}

	//add 
	set<string>::iterator itset = NewDirset.begin();
	for (; itset != NewDirset.end(); itset++)
	{
		GUID guid = CreateGuid();
		//cout << GuidToString(guid).c_str() << endl;
		TiXmlElement Filter("Filter");
		Filter.SetAttribute("Include", itset->c_str());
		TiXmlElement UniqueIdentifier("UniqueIdentifier");
		TiXmlText text(GuidToString(guid).c_str());
		UniqueIdentifier.InsertEndChild(text);
		Filter.InsertEndChild(UniqueIdentifier);
		FirstItemGroup->InsertEndChild(Filter);
	}

	//filepath += ".new";
	doc.SaveFile(filepath.c_str());
	cout << "修改完成！" << endl;
	return 0;
}
