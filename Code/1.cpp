#include <iostream>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

/*GLOBAL VARIABLES*/
int order;//1 for asc and 0 for desc
int debug=1;
long block_size,record_size,input_records,records_in_mem,total_cols,no_of_blocks;
map<string,int> metadata;
vector<string> given_cols;
map<int,int> metadata_bytes_offset;

int print_error(string str)
{
  cout << str << endl;
  return 0;
}
inline bool check_file(const string &file)
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

void get_input_records(char* arr)
{
  string line;
  input_records=0;
  ifstream fs;
  fs.open(arr);
  while(getline(fs,line)){input_records++;}
  // cout << input_records;
  // fs.close();
}
void create_map_metadata()
{
  string col_name,no_of_bytes,line;
  ifstream fs;
  fs.open("metadata.txt");
  record_size=0;
  total_cols=0;
  while(fs>>line)
  {
    total_cols++;
    stringstream iss(line);
    getline(iss,col_name,',');
    getline(iss,no_of_bytes,',');
    record_size+=atoi(no_of_bytes.c_str());
    metadata.insert(make_pair(col_name,atoi(no_of_bytes.c_str())));
  }
  // iss.close();
  record_size+=(metadata.size()-1)*2;
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

bool cmp(const string &f1,const string &f2)
{
  string a="",b="";
  for (map<int,int>::iterator it=metadata_bytes_offset.begin();it!=metadata_bytes_offset.end();it++)
  {
    a+=f1.substr(it->first,it->second-it->first+1);
    b+=f2.substr(it->first,it->second-it->first+1);
  }
  if(!order) {return (a>b);}
  else {return (a<b);}
}
void create_block(vector <string> block_file, int file_no)
{
  int k;
  ostringstream ar;
  ar << file_no;
  string file_name = ar.str() + ".txt";
  sort(block_file.begin(), block_file.end(), cmp);
  ofstream fs(file_name.c_str(), ios_base::app | ios_base::out);
  for(k=0;k<block_file.size();k++)
  {
    fs << block_file[k] << endl;
  }
  fs.close();
}
int main(int argc,char *argv[])
{
  long i;
  if(!check_for_errors(argc,argv))
  {
    return 0;
  }
  else{
    create_map_metadata();
    map<string,int>::iterator iter;
    if(debug){cout << "metadata:" << endl;for(map<string,int>::iterator it=metadata.begin();it!=metadata.end();it++)
      {cout << it->first << " " << it->second << endl;}}
    if(!check_for_error_input(argc,argv))
        return 0;
    create_map_bytes_offset();
    if(debug){cout << "byte_offset:" << endl;for(map<int,int>::iterator it=metadata_bytes_offset.begin();it!=metadata_bytes_offset.end();it++)
      {cout << it->first << " " << it->second << endl;}}
    if(strcmp(argv[4],"desc")==0){order=0;}
    else if(strcmp(argv[4],"asc")==0){order=1;}
    double tot_mem=atoi(argv[3])*1024*1024*0.8;
    get_input_records(argv[1]);
    records_in_mem=(long)floor(tot_mem/record_size);
    no_of_blocks=(long)ceil(input_records*(1.0)/records_in_mem);
    block_size=records_in_mem/(no_of_blocks+1);
    if(debug){
      cout << "input_records:" << input_records<<endl;
      cout << "record_size:"<< record_size<<endl;
      cout << "records_in_mem:"<< records_in_mem<<endl;
      cout << "total_cols:" << total_cols << endl;
      cout << "no_of_blocks:"<< no_of_blocks<<endl;
      cout << "block_size:"<< block_size<<endl;}
    /*PHASE1*/
    vector<string> block_file;
		ifstream fs;
		fs.open(argv[1]);
		int file_no = 0;
    string new_line;
		while(fs != NULL)
		{
			getline(fs, new_line);
			block_file.push_back(new_line);
			if(block_file.size() == records_in_mem)
			{
        file_no++;
        create_block(block_file,file_no);
        block_file.clear();
			}
		}
		block_file.pop_back();
		if(!block_file.empty())
		{
      file_no++;
      create_block(block_file,file_no);
      block_file.clear();
		}
    /*PHASE2*/
    vector < vector<string> > diff_blocks;
    vector <string> file_block;
    vector <int> all_ptr;
    all_ptr.push_back(-1);
    string file_name,file_line;
    for(i=1;i<=no_of_blocks;i++)
    {
      all_ptr.push_back(0);
      ostringstream fs1;
      fs1 << i;
      file_name=fs1.str()+".txt";
      ifstream file_ptr;
      file_ptr.open(file_name.c_str());
      while(getline(file_ptr,file_line)!=NULL)
      {
        file_block.push_back(file_line);
        all_ptr[i]++;
        if(file_block.size()==block_size)
          break;
      }
      file_ptr.close();
      diff_blocks.push_back(file_block);
      file_block.clear();
    }
    int line_cnt;
		string min_record;
		int min_index;
		vector<string> output;

		int cnnt = 0;

		if(check_file(argv[2]))
		{
			remove(argv[2]);
		}

		while(1)
		{
			for(int i=0;i<no_of_blocks;i++)
			{
				if(diff_blocks[i].size() == 0)
				{
					ostringstream abc;
					abc << (i+1);
					file_name = abc.str() + ".txt";
					ifstream file_ptr;
					file_ptr.open(file_name.c_str());

					line_cnt = 0;
					while(getline(file_ptr, file_line))
					{
						if(file_block.size() == block_size || all_ptr[i+1] == records_in_mem)
						{
							break;
						}
						line_cnt++;
						if(line_cnt >= all_ptr[i+1] + 1)
						{
							file_block.push_back(file_line);
						}
					}

					all_ptr[i+1] = all_ptr[i+1] + file_block.size();
					if(file_block.size() != 0)
					{

						diff_blocks[i] = file_block;
					}
					file_block.clear();
					file_ptr.close();
				}
			}
			line_cnt = 0;
			min_record = "";
			for(int i=0;i<no_of_blocks;i++)
			{
				if(diff_blocks[i].size() != 0)
				{
					if(line_cnt == 0)
					{
						min_record = diff_blocks[i].at(0);
						min_index = i;
						line_cnt++;
					}
					else
					{
						min_record = min(min_record, diff_blocks[i].at(0));
						if(strcmp(diff_blocks[min_index].at(0).c_str(), min_record.c_str() ) != 0)
						{
							min_index = i;
						}
					}
				}
			}
			if(strcmp(min_record.c_str(), "")==0)
			{
				break;
			}
			output.push_back(min_record);
			diff_blocks[min_index].erase(diff_blocks[min_index].begin());
			ofstream out_file_pointer;
      out_file_pointer.open(argv[2], ios_base::app);
			if(output.size() == block_size)
			{
				for(int i=0;i<output.size();i++)
				{
					out_file_pointer << output.at(i) << endl;
				}
				output.clear();
			}
			out_file_pointer.close();
		}
		ofstream out_file_pointer;
		//out_file_pointer.open("output.txt", ios_base::app);
		out_file_pointer.open(argv[2], ios_base::app);
		for(int i=0;i<output.size();i++)
		{
			out_file_pointer << output.at(i) << endl;
		}
		out_file_pointer.close();

		string nm;
		for(int i=1;i<=no_of_blocks;i++)
		{
			ostringstream cv;
			cv << i;
			nm = cv.str() + ".txt";
			if(remove(nm.c_str()) != 0)
			{
				cout << "ERROR: Files created for intermediary use are not deleted!!!" << endl;
			}
		}
    if(debug){cout << "working" << endl;}
  }
  return 0;
}
