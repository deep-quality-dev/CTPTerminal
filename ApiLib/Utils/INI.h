/*
   Feather INI Parser - 1.28
   You are free to use this for whatever you wish.

   If you find a bug, please debug the cause and look into a solution.
   Post your compiler, version and the cause or fix in the issues section.

   Written by Turbine.

   Website:
   http://code.google.com/p/feather-ini-parser/downloads

   Help:
   http://code.google.com/p/feather-ini-parser/wiki/Tutorials
*/

#pragma once

//#include <string>
//#include <fstream>
//#include <cstring>
//#include <sstream>
//#include <iostream>
//#include <map>
//#include <stdio.h>
//#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
//

#pragma warning(push)
#pragma warning(disable:4996)

template<class T, class U>
   static T Convert(U value)
{
   stringstream_t sout;
   T result;

   sout << value;
   sout >> result;

   return result;
}

template<>
	static string_t Convert(string_t value)
{
   return value;
}

///
template<class T = string_t, class U = string_t, class V = string_t>
   class INI
{
public:
   typedef TCHAR char_t;
   typedef BYTE uint8_t;
   typedef WORD uint16_t;

   typedef T section_t;
   typedef U key_t;
   typedef V value_t;

   ///Type definitions
   typedef typename std::map<key_t, value_t> keys_t;
   typedef typename std::tr1::shared_ptr<keys_t> keys_t_ptr;
   typedef typename std::map<section_t, keys_t_ptr> sections_t;

   typedef typename keys_t::iterator keysit_t;
   typedef typename sections_t::iterator sectionsit_t;

   typedef typename std::pair<key_t, value_t> keyspair_t;
   typedef typename std::pair<section_t, keys_t_ptr> sectionspair_t;

   ///Settings & Tweaks
   static const int BUFFER_SIZE = 256;

   ///Data members
   keys_t_ptr current;
   sections_t sections;

   ///Constuctor/Destructor
   INI(string_t filename, bool parseFile = true);
   ~INI();

   ///Accessing structure
   keys_t& operator[](section_t section);

   bool create(const section_t section);
   bool select(const section_t section);

   //Set
   bool set(const key_t key, const value_t value);
   template<class W, class X>
      bool set(const W key, const X value)
         { return set(Convert<key_t>(key), Convert<value_t>(value)); }

   //Get
   value_t get(const key_t key, value_t def = value_t());  //Get value, if none exists
   string_t get(const key_t key, const char_t* def = _T(""));  //Can handle const char* without casting

   template<class X, class W>
      X get(const W key, X def = X())
	  { return Convert<X>(get(Convert<key_t>(key), Convert<value_t>(def))); }

   ///Functions
   void nake(const char_t*);  //Strip the line of any non-interpretable characters
   bool parse();
   //bool parseBinary();
   void clear();

   ///Output
   void save(string_t filename = _T(""));
   //void saveBinary(string_t filename = _T(""));

private:
   string_t filename;

   ///Output
   string_t makeSection(const section_t& section);
   string_t makeKeyValue(const key_t& key, const value_t& value);
};

///
template<class T, class U, class V>
   INI<T, U, V>::INI(string_t filename, bool parseFile): filename(filename), current((keys_t*)NULL)
{
   if (parseFile)
      parse();
}

//
template<class T, class U, class V>
   INI<T, U, V>::~INI()
{
   clear();
}

///
template<class T, class U, class V>
   typename INI<T, U, V>::keys_t& INI<T, U, V>::operator[](section_t section)
{
	if (!sections[section].get()) {
		keys_t_ptr temp(new keys_t());
		sections[section] = temp;
	}

	return *sections[section];
}

///
template<class T, class U, class V>
   void INI<T, U, V>::clear()
{
	sections.clear();

	current.reset();
}

///
template<class T, class U, class V>
   bool INI<T, U, V>::create(const section_t section)
{
   if (select(section))
      return false;

   keys_t_ptr temp(new keys_t());
   current = temp;
   sections[section] = current;

   return true;
}

///
template<class T, class U, class V>
   bool INI<T, U, V>::select(const section_t section)
{
   sectionsit_t sectionsit = sections.find(section);
   if (sectionsit == sections.end())
      return false;

   current = sectionsit->second;

   return true;
}

///
template<class T, class U, class V>
   bool INI<T, U, V>::set(const key_t key, const value_t value)
{
   if (current.get() == NULL)
      return false;

   (*current)[key] = value;

   return true;
}

///
template<class T, class U, class V>
   typename INI<T, U, V>::value_t INI<T, U, V>::get(const key_t key, const value_t def)
{
   keysit_t keys = current->find(key);
   if (current == NULL || keys == current->end())
      return def;

   return keys->second;
}


template<class T, class U, class V>
   string_t INI<T, U, V>::get(const key_t key, const char_t* def)
{
   return get(key, (string_t)def);
}

///
template<class T, class U, class V>
   void INI<T, U, V>::nake(const char_t* s)
{
	char_t* ch = (char_t*)s;

	while (*ch != _T('\0')) {
		if (*ch == _T('\n')) {
			*ch = _T('\0');
			break;
		}

		ch++;
	}
}

///
template<class T, class U, class V>
   bool INI<T, U, V>::parse()
{
	current.reset();

	FILE* file;
	_tfopen_s(&file, filename.c_str(), _T("rt"));//_T("r,ccs=UTF-16LE")

	if (!file)
	  return false;

	char_t line[BUFFER_SIZE];

	while(_fgetts(line, _countof(line), file)) {

		nake(line);

		if (_T('\0') == line[0] || line[0] == _T(';') || line[0] == _T('\n')) continue;

		if (_tcslen(line) >= 2 && line[0] == _T('/') && line[1] == _T('/')) continue;


		if (line[0] == _T('[')) {//Section

			stringstream_t out;
			section_t section;

			size_t length = _tcslen(line) - 2;  //Without section brackets
			while(isspace(line[length + 1]))  //Leave out any additional new line characters
			  --length;

			string_t ssection;
			ssection.assign(&line[1], length);

			keys_t_ptr temp(new keys_t());
			current = temp;

			if (typeid(section_t) == typeid(string_t)) {
				section = ssection;
			} else {
				out << ssection;
				out >> section;
			}

			sections[section] = current;
		
		} else  {//Key

			stringstream_t out1, out2;
			key_t key;
			value_t value;

			char_t* skey;
			char_t* svalue;

			skey = _tcstok(line, _T("="));
			svalue = _tcstok(NULL, _T(";"));

			if (skey != NULL && svalue != NULL) {
			   out1 << skey;
			   out1 >> key;

			   if (typeid(value_t) == typeid(string_t)) {
				   value = svalue;
			   } else {
				   out2 << svalue;
				   out2 >> value;
			   }

			  (*current)[key] = value;
			}
		}
	}

   fclose(file);

   return true;
}

///GOES HERE PARSE

///
template<class T, class U, class V>
   string_t INI<T, U, V>::makeSection(const section_t& section)
{
   stringstream_t line;
   line << _T('[') << section << _T(']') << std::endl;

   return line.str();
}

///
template<class T, class U, class V>
   string_t INI<T, U, V>::makeKeyValue(const key_t& key, const value_t& value)
{
   stringstream_t line;
   line << key << _T('=') << value << std::endl;

   return line.str();
}

///
template<class T, class U, class V>
   void INI<T, U, V>::save(string_t filename)
{
   filename = (filename.empty()) ? (this->filename) : (filename);

   FILE* file;
   _tfopen_s(&file, filename.c_str(), _T("w"));//_T("w,ccs=UTF-16LE")

   //Loop through sections
   for(INI::sectionsit_t i = sections.begin(); i != sections.end(); i++)
   {
	   if (i->second->size() == 0)  //No keys/values in section, skip to next
		   continue;

      //Write section
      const string_t temp = makeSection(i->first);
      const char_t* line = temp.c_str();

	  _fputts(line, file);

      for(INI::keysit_t j = i->second->begin(); j != i->second->end(); j++)
      {
         //Write key and value
         const string_t temp = makeKeyValue(j->first, j->second);
         const char_t* line = temp.c_str();
          _fputts(line, file);
      }
   }

   fclose(file);
}

///
//template<class T, class U, class V>
//   bool INI<T, U, V>::parseBinary()
//{
//   current = NULL;
//   std::ifstream file(filename.c_str(), ifstream_t::binary);
//   if (!file.is_open())
//      return false;
//
//   sections.clear();
//
//   uint16_t sectionCount;
//   uint8_t keyCount;
//   key_t key;
//   value_t value;
//   section_t section;
//   uint8_t ss;
//
//   file.read((char*)&sectionCount, sizeof(sectionCount));
//
//   for(uint32_t i = 0; i < sectionCount; i++)
//   {
//      if (i > 0)
//         file.seekg(file.tellg() + 1);
//
//      file.read((char*)&keyCount, sizeof(keyCount));
//      file >> section;
//
//      create(section);
//
//      for(uint8_t j = 0; j < keyCount; j++)
//      {
//         file >> key;
//         file >> value;
//         set(key, value);
//      }
//   }
//
//   file.close();
//
//   return true;
//}
//
/////
//template<class T, class U, class V>
//   void INI<T, U, V>::saveBinary(string_t filename)
//{
//   ofstream_t file(((filename == _T(""))? this->filename: filename).c_str(), ofstream_t::trunc | ofstream_t::binary);
//   uint16_t sectionCount = sections.size();
//   uint8_t keyCount;
//
//   file.write((char*)&sectionCount, sizeof(sectionCount));
//
//   //Loop through sections
//   for(INI::sectionsit_t i = sections.begin(); i != sections.end(); i++)
//   {
//      keyCount = i->second->size();
//      file.write((char*)&keyCount, sizeof(keyCount));
//
//      file << i->first << endl;
//
//      for(INI::keysit_t j = i->second->begin(); j != i->second->end(); j++)
//      {
//         file << j->first << endl;
//         file << j->second << endl;
//      }
//   }
//
//   file.close();
//}

#pragma warning(pop)