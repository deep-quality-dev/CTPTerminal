#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMMENT		'#'
#define INI_ERROR	-1

typedef enum
{
	REPLACE = 1,
	NON_REPLACE = 0
}REPLACE_FLAG;

struct record
{
	char comments[255];
	char key[255];
	char value[255];
	struct record *next;
};

/************************************************************************/
/* for content of file                                                  */
/************************************************************************/
struct section
{
	struct record *data_first;
	struct record *data_last;
	unsigned int data_size;
	char comments[255];
	char name[255];
	struct section *next;
};
/************************************************************************/
/* for content list                                                     */
/************************************************************************/
typedef struct
{
	unsigned int section_size;
	struct section *first;
	struct section *last;
}content;

// global variable [10/18/2005]
extern content *ini_content;
extern char gfilename[255];
extern char *error_msg;
extern REPLACE_FLAG w_flag;

class CIniManager
{
public:
	CIniManager(void);
	~CIniManager(void);
	bool ini_start(const char* filename);
	bool load(const char *filename);
	bool save();	// save to load filebool 
	bool save_as(const char *filename);
	char *get_value (const char *sec,const char *key);
	char *_get_value(const char *sec,const char *key,		// return data and comment
		char *comment);
	bool set_value  (const char *sec,const char *key,		// will auto replace
		const char *value);	
	bool _set_value (const char *sec,const char *key,		// select replace or not replace
		const char *value,const char *comment,REPLACE_FLAG flag);

	int  remove_sel (const char *sec,char *key);
	int  remove_all (const char * sec);				// remove all record in section
	void add_section(const char *sec,const char *comment);	// add section
	int  remove_section(char *sec);				// remove section (remove all record in section if not empty)

	void clear();								// clear all content

	// size of section
	int  content_size();
	int  section_size(char *sec);
	void ini_end();
	void _ini_end(REPLACE_FLAG flag);
	// initaial data/save
	void init_content();
	bool _save(const char *filename);
	// add/remove record
	void _append(const char *sec,const char *key,			// append data to section
		const char *value,const char *comment);											
	int  _remove(const char *sec,const char *key);
	int  _remove_all(struct section *sec);
	// display function
	void _print_allrecord(struct record *sec);	// print all record in section
	// search section
	struct section *get_section(const char *sec);		// search section
	struct record *get_record(struct section *sec,const char *key);	// get record
	// trim
	void trim(char *buffer);
};
