#include <iostream>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <sstream>


using namespace std;

/*GLOBAL VARIABLES*/
int order;//0 for asc and 1 for desc
int debug=1;
map<string,int> metadata;
vector<string> given_cols;
map<int,int> metadata_bytes_offset;

int print_error(string str)
{
  cout << str << endl;
  return 0;
}
bool check_file(const string &file)
{
  struct stat buffer;
  return (stat(file.c_str(),&buffer)==0);
}
int check_for_errors(int argc,char *argv[])
{
  if(argc <=5 )
  {
    return print_error("Error:less argument given");
  }
  else if(check_file("metadata.txt")==0)
  {
    return print_error("Error:metadata.txt file doesn't exist");
  }
  else if(check_file(argv[1])==0)
  {
    return print_error("Error:input.txt file doesn't exist");
  }
  else if(strcmp(argv[4],"desc")!=0 && strcmp(argv[4],"asc")!=0)
  {
    return print_error("Error:given argument for the asc and desc is not in correct place.");
  }
  return 1;
}

void create_map_metadata()
{
  string col_name,no_of_bytes,line;
  ifstream fs("metadata.txt");
  while(getline(fs,line))
  {
    stringstream iss(line);
    getline(iss,col_name,',');
    getline(iss,no_of_bytes,',');
    metadata.insert(make_pair(col_name,atoi(no_of_bytes.c_str())));
  }
  fs.close();
}

int check_for_error_input(int argc,char* argv[])
{
  int i;
  for(i=5;i<argc;i++)
  {
    given_cols.push_back(argv[i]);
  }
  //check if not present in metadata
  for(i=0;i<given_cols.size();i++)
  {
    if(metadata.find(given_cols[i])==metadata.end())
    {
      print_error("Error:the input cols in command are wrong");
      return 0;
    }
  }
  return 1;
}

void create_map_bytes_offset()
{
  int i,offset=0;
  map<string,int>::iterator iter;
  for(i=0;i<given_cols.size();i++)
  {
    for(iter=metadata.begin();iter!=metadata.end();iter++)
    {
      if(offset!=0){
        offset+=2;
      }
      string s1=iter->first,s2=given_cols[i];
      if(strcmp(s1.c_str(),s2.c_str())==0)
      {
        metadata_bytes_offset.insert(make_pair(offset,offset+iter->second-1));
        break;
      }
      offset+=iter->second;
    }
    offset=0;
  }
}

int main(int argc,char *argv[])
{
  if(!check_for_errors(argc,argv))
  {
    return 0;
  }
  else{
    create_map_metadata();
    if(debug){cout << "metadata:" << endl;for(auto it=metadata.begin();it!=metadata.end();it++)
      {cout << it->first << " " << it->second << endl;}}
    if(!check_for_error_input(argc,argv))
        return 0;
    create_map_bytes_offset();
    if(debug){cout << "byte_offset:" << endl;for(auto it=metadata_bytes_offset.begin();it!=metadata_bytes_offset.end();it++)
      {cout << it->first << " " << it->second << endl;}}
    if(strcmp(argv[4],"desc")==0){order=0;}
    else if(strcmp(argv[4],"asc")==0){order=1;}


    if(debug){cout << "working" << endl;}
  }
  return 0;
}
