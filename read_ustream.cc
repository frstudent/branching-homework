
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
class Parser
{
  enum states_t 
  { 
    st_error,
    st_startup, 
    st_check_attribute,
    st_processing_instruction,
    st_extension_state,
    st_close_tat_state,
    st_collect_tag,
    st_check_attrib,
    st_collect_attrib,
    st_start_atribute_value,
    st_collect_attribute_value,
  };

  states_t	state;
  wchar_t	ch;
  wstring	tag;
  wstring	attrib;
  wstring	value;

  void Distinguish()
  {
    switch(ch)
    {
      case L'?':
        state = st_processing_instruction;
        break;
      case L'!':
        state = st_extension_state;
        break;
      case L'/':
        state = st_close_tat_state;
        break;
      default:
        if(isalpha(ch))
        {
          tag = ch;
          state =  st_collect_tag;
        }
        else
          state = st_error;
        break;
    }
  }

  void CollectTag()
  {
    if(isspace(ch))
      state = st_check_attrib;
    else
      tag += ch;
  }

  void CheckAttrib()
  {
     if(!isspace(ch))
     {
       if(isalpha(ch))
       {
         attrib = ch;
         state = st_collect_attrib;
       }
       else
         state = st_error;
     }
  }

  void CollectAttrib()
  {
     if(ch == L'=')
     {
        value = L"";
        state = st_start_atribute_value;
     }
     else if(isalnum(ch))
       attrib += ch;
     else
       state = st_error;
  }

public:
  bool Fetch(wchar_t ch)
  {
    this->ch = ch;
    switch(state)
    {
      case st_startup:
        if(ch == L'<')
          state = st_check_attribute;
        else
          state = st_error;
        break;
      case st_check_attribute:
        Distinguish();
        break;
      case st_collect_tag:
        CollectTag();
        break;
      case st_check_attrib:
        CheckAttrib();
        break;
      case st_collect_attrib:
        CollectAttrib();
        break;
      case st_start_atribute_value:
        if(ch == L'"')
          state = st_collect_attribute_value;
        else
          state = st_error;
      default:
        cout << "State not parsed" << endl;
    }
    wcout << ch;
    return state != st_error;
  }

  Parser()
  {
    state = st_startup;
  }
};

int load(const char * filename)
{
  int		status = 0;
  wifstream	report;
  Parser	parser;
  wchar_t	ch;

  report.open(filename);
  if(!report)
    return -1;
  while(report.get(ch))
    if(!parser.Fetch(ch))
    {
       cerr << "Input file format error" << endl;
       status = 1;
       break;
    }
  report.close();
  return status;
}

int main(int argc, char * argv[])
{
  if(argc < 2)
  {
    cerr << "Not enough program arguments" << endl;
    return -1;
  }
  load(argv[1]);
  return 0;
}
