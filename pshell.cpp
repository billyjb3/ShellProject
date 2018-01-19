#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <sys/wait.h>
#include <ctime>

using namespace std;

vector<string> split(const string& s);

struct fileblock
{
	string name;
	string date;
};
struct dir
{
	string name;
	string date;
	dir *parent;
	vector<dir*> children; 
	vector<fileblock*> files;
};


int main()
{
	cout<<"Welcome to DDBShell! type exit to quit"<<endl;
	
	dir *cwd; //current working directory
	dir *root; //root directory
	string iline; //command line input;
	
	int setup = 0; //if fs has been initialized or not.
	
	string pbase = "DDBShell:";
	
	//keeps looping until exit command is entered
	//every loop takes the console input, splits it into words, and checks for command uses
	while(1)
	{
		cout<< pbase;
		getline(cin, iline);
		vector<string> input = split(iline);
		
		/*
		exit command
		*/
		if(input[0] == "exit")
		{
			cout<<"Goodbye!"<<endl;
			return 0;
		}
		
		/*
		mkfs command
		*/
		else if(input[0] == "mkfs") 
		{
			//if first mkfs call, create root directory
			if(setup == 0) 
			{
				cwd = new dir;
				root = cwd;
				cwd -> name = "root";
				setup = 1;
				cout<<"Filesystem is ready for use"<<endl;
			}
			//if called after fs creation, delete current content of fs
			//deletes files and directories in a recursive bottom up approach
			else 
			{
				cwd = root;
				while(root->children.size() != 0)
				{
					//check current dir for children
					if(cwd->children.size() != 0) 
					{	
						//check to see if first child has children
						//if so change cwd to cwd's first child; loop
						if(cwd->children[0]->children.size() != 0)
						{
							cwd = cwd->children[0];
						}
						//if not, delete first child and its files; loop
						else
						{
							//delete files in dir first
							while(cwd->children[0]->files.size() != 0)
							{
								cout<<cwd->children[0]->files[0]->name<<" deleted"<<endl;
								delete cwd->children[0]->files[0];
								cwd->children[0]->files.erase(cwd->children[0]->files.begin());
							}
							//delete dir
							cout<<cwd->children[0]->name<<" deleted"<<endl;
							delete cwd->children[0];
							cwd->children.erase(cwd->children.begin());
						}
					}
					//if cwd has no children, change to parent; loop
					else
					{
						cwd = cwd->parent;
					}
				}
				//delete any files in root dir
				while(root->files.size() != 0)
				{
					cout<<root->files[0]->name<<" deleted"<<endl;
					delete root->files[0];
					root->files.erase(root->files.begin());
				}
				cout<<"Filesystem is ready for use"<<endl;
			} 
		}
		
		/*
		check to make sure mkfs has been called at least once before preceding
		*/
		else if(setup == 0) 
		{
			cout<<"must use mkfs first to setup file system"<<endl;
		}
		
		/* 
		mkdir command
		*/
		else if(input[0] == "mkdir")
		{
			if(input.size() == 2)
			{
				int match = 0;
				//check for matching dir name
				for(int i = 0; i < cwd->children.size(); i++)
				{
					if(cwd->children[i]->name == input[1])
					{
						cout<<"That name already exists"<<endl;
						match = 1;
					}
				}
				//check for matching file name
				for(int i = 0; i < cwd->files.size(); i++)
				{
					if(cwd->files[i]->name == input[1])
					{
						cout<<"That name already exists"<<endl;
						match = 1;
					}
				}
				//if no matches found make dir
				if(match == 0)
				{
					cwd -> children.push_back(new dir);	
					cwd -> children.back() -> name = input[1];
					cwd -> children.back() -> parent = cwd;
					time_t t = time(0);
					string st = ctime(&t);
					cwd -> children.back() -> date = st;
					cout<<"directory: ["<<input[1]<<"] created at "<<st;
				}
			}
			//useage error
			else
			{
				cout<<"Useage: mkdir <dirname>"<<endl;
			}
		} 
		
		/*
		mkfile command
		*/
		else if(input[0] == "mkfile")
		{
			if(input.size() == 2)
			{
				//check for matching dir name
				int match = 0;
				for(int i = 0; i < cwd->children.size(); i++)
				{
					if(cwd->children[i]->name == input[1])
					{
						cout<<"That name already exists"<<endl;
						match = 1;
					}
				}
				//check for matching file name
				for(int i = 0; i < cwd->files.size(); i++)
				{
					if(cwd->files[i]->name == input[1])
					{
						cout<<"That name already exists"<<endl;
						match = 1;
					}
				}
				//if no matches found, make file
				if(match == 0)
				{
					cwd -> files.push_back(new fileblock);
					cwd -> files.back() -> name = input[1];
					time_t t = time(0);
					string st = ctime(&t);
					cwd -> files.back() -> date = st;
					cout<<"file: "<<input[1]<<" created at "<<st;
				}
			}
			//useage error
			else
			{
				cout<<"Useage: mkfile <filename>"<<endl;
			}
		}
		
		/*
		ls command
		*/
		else if(input[0] == "ls")
		{
			//list directories
			for(int i = 0; i < cwd->children.size(); i++)
			{
				cout<<"["<<cwd->children[i]->name<<"]	";
			}
			//list files
			for(int i = 0; i < cwd->files.size(); i++)
			{
				cout<<cwd->files[i]->name<<"	";
			}
			cout<<endl;
		}
		
		/*
		stat command
		*/
		else if(input[0] == "stat")
		{
			if(input.size() ==2)
			{
				int found = 0;
				//search directories for match
				for(int i = 0; i < cwd->children.size(); i++)
				{
					if(cwd->children[i]->name == input[1])
					{
						cout<<cwd->children[i]->name<<" created: "<<cwd->children[i]->date<<endl;
						found = 1;
					}
				}
				//search files for match
				for(int i = 0; i < cwd->files.size(); i++)
				{
					if(cwd->files[i]->name == input[1])
					{
						cout<<cwd->files[i]->name<<" created: "<<cwd->files[i]->date<<endl;
						found = 1;
					}
				}
				//file/dir not found error
				if(found == 0)
				{
					cout<<input[1]<<" not found"<<endl;
				}
			}
			//useage error
			else
			{
				cout<<"Useage: stat <fileorDirName>";
			}
		}
		
		/*
		rmdir command
		*/
		else if(input[0] == "rmdir")
		{
			if(input[1] == "root")
			{
				cout<<"Can't delete root directory!"<<endl;
			}
			else if(input.size() == 2)
			{
				//first check for matching file name
				int isfile = 0;
				for(int i = 0; i < cwd->files.size(); i++)
				{
					if(cwd->files[i]->name == input[1])
					{
						cout<<input[1]<<" is a file name"<<endl;
						isfile = 1;
					}
				}
				//if no matching file name proceed
				if(isfile == 0)
				{
					//check to see if dir exists
					int found = 0;
					for(int i = 0; i < cwd->children.size(); i++)
					{
						if(cwd->children[i]->name == input[1])
						{
							found = 1;
							//make sure dir is empty
							if(cwd->children[i]->children.size() != 0)
							{
								cout<<"cant delete non-empty directories!"<<endl;
							}
							//delete!
							else
							{
								delete cwd->children[i];
								cwd->children.erase(cwd->children.begin() + i);
							}
						}
					}
					//if not found error
					if(found == 0)
					{
						cout<<input[1]<<" not found"<<endl;
					}
				}	
			}
			//useage error
			else
			{
				cout<<"Useage: rmdir <dirname>"<<endl;
			}
		}
		
		/*
		rmfile command
		*/
		else if(input[0] == "rmfile")
		{
			if(input.size() == 2)
			{
				//first check for matching dir name
				int isdir = 0;
				for(int i = 0; i < cwd->children.size(); i++)
				{
					if(cwd->children[i]->name == input[1])
					{
						cout<<input[1]<<" is a directory name!"<<endl;
						isdir = 1;
					}
				}
				//if no matching dir name proceed
				if(isdir == 0)
				{
					int found = 0;
					for(int i = 0; i < cwd->files.size(); i++)
					{
						if(cwd->files[i]->name == input[1])
						{
							found = 1;
							delete cwd->files[i];
							cwd->files.erase(cwd->files.begin() + i);
						}
					}
					//file not found error
					if(found == 0)
					{
						cout<<input[1]<<" not found!"<<endl;
					}
				}
			}
			//useage error
			else
			{
				cout<<"Useage: rmfile <filename>"<<endl;
			}
		}
		
		/*
		cd command
		*/
		else if(input[0] == "cd")
		{
			if(input.size() == 2)
			{
				//change to parent, if not in root dir
				if(input[1] == "..")
				{
					if(cwd->name != "root")
					{
						cwd = cwd->parent;
					}
				}
				else
				{
					int found = 0;
					for(int i = 0; i < cwd->children.size(); i++)
					{
						if(cwd->children[i]->name == input[1])
						{
							found = 1;
							cwd = cwd->children[i];
						}
					}
					//dir not found error
					if(found == 0)
					{
						cout<<input[1]<<" not found"<<endl;
					}	
				}
			}
			//useage error
			else
			{
				cout<<"Useage: cd <dirname>"<<endl<<"cd .. for parent"<<endl;
			}
		}
		
		/*
		no command matches; command not found print out
		*/
		else
		{
			cout<<"command "<<iline<<" not found"<<endl;
			iline = "";
		}
	}
}

/*
string splitter function
*/
vector<string> split(const string& s)
{
	vector<string> sps; 
	typedef string::size_type string_size; 
	string_size i = 0;
	
	while(i != s.size())
	{
		while(i != s.size() && isspace(s[i]))
		{
			i++;
		}
		string_size j = i;
		while(j != s.size() && !isspace(s[j]))
		{
			j++;
		}
		if(i != j)
		{
			sps.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return sps;
}
