#include "IniManager.h"

content *ini_content;
char gfilename[255];
char *error_msg;
REPLACE_FLAG w_flag;

CIniManager::CIniManager(void)
{
}

CIniManager::~CIniManager(void)
{
	if (ini_content != NULL)
	{
		free(ini_content);
		ini_content = NULL;
	}
}
bool CIniManager::ini_start(const char* filename)
{
	init_content();

	error_msg = NULL;
	w_flag = REPLACE;
	memcpy(gfilename,filename,sizeof(gfilename));

	if(load(gfilename) == false)
	{
		error_msg = "initial parse file error";
		return false;
	};
	return true;
}
void CIniManager::_ini_end(REPLACE_FLAG flag)
{
	if(flag == REPLACE)
		save();
	clear();
}

//////////////////////////////////////////////////////////////////////////
// ini end
void CIniManager::ini_end()
{
	_ini_end(REPLACE);
}

//////////////////////////////////////////////////////////////////////////
// load
//  [10/18/2005]
bool CIniManager::load(const char *filename)
{
	FILE *in_stream;
	char buffer[255];
	char comments[1024];
	char current_section[255];	
	char key[255];
	char value[255];
	char *pdest;
	int  index;

	strcpy_s(comments,1024,"");
	strcpy_s(current_section,255,"");
	error_msg = NULL;

	fopen_s(&in_stream, filename, "r");
	if (in_stream != NULL)
	{
		while(fgets(buffer,sizeof(buffer),in_stream) != NULL)
		{
			trim(buffer);
			switch(buffer[0])
			{
			case '[' : // section;
				pdest = strrchr(buffer,']');
				if (pdest == NULL)
				{
					fclose(in_stream);
					error_msg = "parse ini error";
					return false;
				}
				index = pdest - buffer;
				memcpy(current_section,buffer + 1,index - 1);
				current_section[index - 1] = '\0';
				add_section(current_section,comments);	
				strcpy_s(comments,"");
				break;
			case '#' : // comment
			case ';' :
				if(strlen(comments) > 0)
					strcat_s(comments,"\n");
				strcat_s(comments,buffer);
				break;
			case '\0':
				break;
			default : // find content
				pdest = strrchr(buffer,'=');
				if (pdest == NULL) 
				{
					fclose(in_stream);
					error_msg = "parse ini error";
					return false;
				}
				index = pdest - buffer;
				memcpy(key,buffer,index);
				key[index] = '\0';
				memcpy(value,buffer + index + 1,strlen(buffer)-index);

				if(strcmp(current_section,"") == 0)
				{
					fclose(in_stream);
					error_msg = "parse ini error";
					return false;
				}
				else
				{
					_append(current_section,key,value,comments);
					strcpy_s(comments,"");
				}
				break;
			}
		}
		fclose(in_stream);
	}
	else
	{
		error_msg = "open file error";
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// _save
//  [10/18/2005]
bool CIniManager::_save(const char *filename)
{
	FILE *stream;
	struct section *sec = ini_content->first;
	struct record *rec;	

	error_msg = NULL;

	fopen_s(&stream, filename, "w");
	if (stream == NULL )
	{
		error_msg = "open file error";
		return false;
	}

	while (sec != NULL)
	{
		if(strlen(sec->comments) != 0)
		{
			fprintf(stream,"%s\n",sec->comments);		
		}
		fprintf(stream,"[%s]\n",sec->name);
		// print section content
		rec = sec->data_first;
		while(rec != NULL)
		{
			if(strlen(rec->comments) != 0)
			{
				fprintf(stream,"%s\n",rec->comments);				
			}
			fprintf(stream,"%s=%s\n",rec->key,rec->value);

			rec = rec->next;
		}		
		sec = sec->next;		
	}	
	fclose(stream);
	return true;
}
//////////////////////////////////////////////////////////////////////////
// save to default file
// [10/18/2005]
bool CIniManager::save()
{
	return _save(gfilename);
}

//////////////////////////////////////////////////////////////////////////
// save as (for users)
// [10/19/2005]
bool CIniManager::save_as(const char *filename)
{
	return _save(filename);
}

//////////////////////////////////////////////////////////////////////////
// get value
// [10/19/2005]
char* CIniManager::get_value(const char *sec,const char *key)
{
	struct record *result =	get_record(get_section(sec),key);	
	if(result != NULL)
		return result->value;
	else
		return "";
}
//////////////////////////////////////////////////////////////////////////  
// [10/19/2005]
char* CIniManager::_get_value(const char *sec,const char *key,  char *comment)
{
	struct record *result =	get_record(get_section(sec),key);	
	if(result != NULL)
	{
		strcpy_s(comment,256,result->comments);
		return result->value;
	}	
	else
	{
		strcpy_s(comment, 256, "");
		return "";
	}
}

//////////////////////////////////////////////////////////////////////////
// set value if exist will be replace
// [10/20/2005]
bool CIniManager::set_value(const char *sec,const char *key,  const char *value)
{
	return _set_value(sec,key,value,"",REPLACE);	
}
//////////////////////////////////////////////////////////////////////////
// [10/20/2005]
bool CIniManager::_set_value(const char *sec,const char *key,  const char *value,const char *comment,REPLACE_FLAG flag)
{
	w_flag = flag;
	error_msg = NULL;
	_append(sec,key,value,comment);
	if(error_msg == NULL)
		return true;
	else
		return false;
}

/************************************************************************/
/* helper function section                                              */
/************************************************************************/
//////////////////////////////////////////////////////////////////////////
// init list of ini file
//  [10/18/2005]
void CIniManager::init_content()
{
	ini_content = (content *)malloc(sizeof(content));

	if(ini_content == NULL)
	{
		error_msg = "cannot malloc memory !";
		return;
	}

	ini_content->section_size = 0;	
	ini_content->first = NULL;
	ini_content->last = NULL;
}

//////////////////////////////////////////////////////////////////////////
// add section
//  [10/18/2005]
void CIniManager::add_section(const char *sec,const char *comment)
{
	struct section *temp;
	temp = get_section(sec);

	error_msg = NULL;

	if(temp == NULL)
	{
		temp = (struct section *)malloc(sizeof(struct section));

		if(temp == NULL)
		{
			error_msg = "cannot malloc memory !";
			return;
		}

		// for section name
		strcpy_s(temp->name,sec);

		if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
			sprintf_s(temp->comments,"#%s",comment);
		else
			strcpy_s(temp->comments,comment);

		// for data link
		temp->data_first = NULL;
		temp->data_last = NULL;
		temp->next = NULL;
		temp->data_size = 0;

		// increment section size
		ini_content->section_size++;

		// for content link
		if (ini_content->first == NULL)
		{
			ini_content->first = temp;
			ini_content->last  = temp;
		}
		else
		{
			ini_content->last->next = temp;
			ini_content->last = temp;
		}	
	}
	else if(w_flag == REPLACE)
	{


		strcpy_s(temp->name,sec);
		if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
			sprintf_s(temp->comments,"#%s",comment);
		else
			strcpy_s(temp->comments,comment);
	}
}

//////////////////////////////////////////////////////////////////////////
// append list
//  [10/18/2005]
void CIniManager::_append(const char *sec,const char *key,const char *value,const char *comment)
{
	struct section *tmp_sec;
	struct record *temp;	

	// find section
	tmp_sec = get_section(sec);

	if(tmp_sec != NULL)
	{
		temp = get_record(tmp_sec,key);
		if(temp == NULL)
		{
			temp = (struct record *)malloc(sizeof(struct record));
			if(temp == NULL)
			{
				error_msg = "cannot malloc memory !";
				return;
			}
			temp->next = NULL;	

			if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
				sprintf_s(temp->comments,"#%s",comment);
			else
				strcpy_s(temp->comments,comment);
			strcpy_s(temp->key,key);
			strcpy_s(temp->value,value);			
			tmp_sec->data_size++;

			if (tmp_sec->data_first == NULL)
			{
				tmp_sec->data_first = temp;
				tmp_sec->data_last  = temp;
			}
			else
			{
				tmp_sec->data_last->next = temp;
				tmp_sec->data_last = temp;
			}			
		}
		else if(w_flag == REPLACE)
		{
			if((comment[0] != '#' || comment[0] != ';') && (strlen(comment) > 0))
				sprintf_s(temp->comments,"#%s",comment);
			else
				strcpy_s(temp->comments,comment);

			strcpy_s(temp->key,key);
			strcpy_s(temp->value,value);
		}

	}
	else
	{
		add_section(sec,"");
		_append(sec,key,value,comment);
	}
}


//////////////////////////////////////////////////////////////////////////
// search and get section
//  [10/18/2005]
struct section* CIniManager::get_section(const char *sec)
{
	bool found = false;
	struct section *esection = ini_content->first;
	while (esection != NULL)
	{	
		if(strcmp(esection->name,sec) == 0)
		{
			found = true;
			break;
		}		
		esection = esection->next;
	}

	if(found == true)
		return esection;
	else
		return NULL;
};

//////////////////////////////////////////////////////////////////////////
// search and get record
struct record* CIniManager::get_record(struct section *sec,const char *key)
{
	bool found = false;
	struct record *tmp;

	if(sec == NULL)
		return NULL;

	tmp = sec->data_first;

	while(tmp != NULL)
	{
		if(strcmp(key,tmp->key) == 0)
		{
			found = true;
			break;
		}
		tmp = tmp->next;
	}

	if(found == true)
	{
		return tmp;
	}
	else
	{
		return NULL;
	}
};

//////////////////////////////////////////////////////////////////////////
// remove list //return num of remove 0 nothing to remove 1 is success
//  [10/18/2005]
int CIniManager::_remove(const char *sec,const char *key)
{	
	struct section *temp_sec = get_section(sec);
	struct record *tmp,*tmp2;
	int	remove = 0;

	if(temp_sec == NULL)
		return 0;

	tmp = temp_sec->data_first;

	if(tmp == NULL)
		return 0;

	if(strcmp(key,tmp->key) == 0)
	{
		temp_sec->data_first = tmp->next;
		temp_sec->data_size--;
		free(tmp);
		return 1;
	}

	while(tmp != NULL)
	{
		if(tmp->next != NULL)
		{
			if(strcmp(key,tmp->next->key) == 0)
			{	
				tmp2 = tmp->next;				
				tmp->next = tmp->next->next;
				temp_sec->data_size--;
				free(tmp2);				
				remove = 1;
				break;
			}
		}		
		tmp = tmp->next;
	}		
	return remove;
}

//////////////////////////////////////////////////////////////////////////
// remove all record
// [10/18/2005]
int CIniManager::remove_all(const char *sec)
{
	struct section *temp_sec = get_section(sec);
	struct record *tmp;
	int remove = 0;

	if(temp_sec == NULL)
		return 0;

	tmp = temp_sec->data_first;
	while(tmp != NULL)
	{
		temp_sec->data_first = tmp->next;
		temp_sec->data_size--;
		free(tmp);
		remove++;
		tmp = temp_sec->data_first;
	}
	return remove;
}
//////////////////////////////////////////////////////////////////////////
// remove selection record
int CIniManager::remove_sel(const char *sec,char *key)
{
	return _remove(sec,key);
}

//////////////////////////////////////////////////////////////////////////
// remove all record
// [10/18/2005]
int  CIniManager::_remove_all(struct section *sec)
{
	struct record *tmp;
	int remove = 0;

	if(sec == NULL)
		return 0;

	tmp = sec->data_first;
	while(tmp != NULL)
	{
		sec->data_first = tmp->next;
		sec->data_size--;
		free(tmp);
		remove++;
		tmp = sec->data_first;
	}
	return remove;
}

//////////////////////////////////////////////////////////////////////////
// remove section
// [10/18/2005]
int  CIniManager::remove_section(char *sec)
{
	struct section *esection = ini_content->first,*temp;
	int remove = 0;

	if(esection == NULL)
		return 0;

	if(strcmp(sec,esection->name) == 0)
	{
		_remove_all(esection);
		ini_content->first = esection->next;
		ini_content->section_size--;
		free(esection);
		return 1;
	}

	while (esection != NULL)
	{	
		if(strcmp(esection->next->name,sec) == 0)
		{
			_remove_all(esection->next);
			temp = esection->next;				
			esection->next = esection->next->next;
			ini_content->section_size--;
			free(temp);				
			break;
		}		
		esection = esection->next;
	}		
	return remove;
}

//////////////////////////////////////////////////////////////////////////
// clear all content
//  [10/18/2005]
void CIniManager::clear()
{
	struct section *tmp;
	if(ini_content == NULL)
		return;

	tmp = ini_content->first;
	while(tmp != NULL)
	{
		ini_content->first = tmp->next;
		ini_content->section_size--;
		_remove_all(tmp);
		free(tmp);
		tmp = ini_content->first;
	}	

	if (ini_content != NULL) {
		free(ini_content);
		ini_content = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// print all content
//  [10/18/2005]
//////////////////////////////////////////////////////////////////////////
// get size of content (number of section
//  [10/18/2005]
int CIniManager::content_size()
{
	return ini_content->section_size;
}

//////////////////////////////////////////////////////////////////////////
// get size of selection section
//  [10/18/2005]
int CIniManager::section_size(char *sec)
{
	struct section *temp = get_section(sec);
	return temp->data_size;
}

//////////////////////////////////////////////////////////////////////////
// trime ' ' \n\t\r
//  [10/19/2005]
void CIniManager::trim(char *buffer)
{
	if(buffer[strlen(buffer)-1] == '\n')
		buffer[strlen(buffer)-1] = '\0';
	//char temp[1024];
	//if(buffer[0] )
}
