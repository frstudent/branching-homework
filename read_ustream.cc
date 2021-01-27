
#include <iostream>
#include <fstream>
#include <string>
#include <list>

using namespace std;

class D3D_Attrib
{
public:
    wstring Name;
    wstring Value;
    D3D_Attrib(wstring name, wstring value)
    {
        Name = name;
        Value = value;
    }
};

class D3D_Node
{
public:
    wstring                 NodeName;
    D3D_Node            *   Parent;
    list<D3D_Attrib *>      Attributes;
    list<D3D_Node *>        Child;

    D3D_Node(wstring name, D3D_Node * parent)
    {
        NodeName = name;
        Parent = parent;
    }
};

class Parser
{
  enum states_t 
  { 
    st_error,
    st_startup, 
    st_signature_2,
    st_signature_3,
    st_default_state,
    st_check_tag,
    st_processing_instruction,
    st_extension_state,
    st_close_tag_state,
    st_collect_tag,
    st_check_attrib,
    st_collect_attrib,
    st_start_atribute_value,
    st_collect_attribute_value,
    st_check_delimiter,
    st_expect_close_tag,
  };

  states_t	state;
  wchar_t	ch;
  wstring	tag;
  wstring	attrib;
  wstring	value;
  int       line_nuber;
  D3D_Node  *   root;
  D3D_Node  *   current;

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
        state = st_close_tag_state;
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
      if (isspace(ch))
      {
          current = new D3D_Node(tag, current);
          tag.clear();
          state = st_check_attrib;
      }
      else if (ch == L'/')
      {
          current = new D3D_Node(tag, current);
          tag.clear();
          state = st_expect_close_tag;
      }
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
     else if(ch == L'.' | isalnum(ch))
       attrib += ch;
     else
       state = st_error;
  }

  void CollectValue()
  {
    if(ch != L'"')
      value += ch;
    else
    {
      D3D_Attrib * attr = new D3D_Attrib(this->attrib, this->value);
      this->attrib.clear();
      this->value.clear();
      current->Attributes.push_back(attr);
      state = st_check_delimiter;
    }
  }

  void DeafaultState()
  {
    if(ch == L'<')
      state = st_check_tag;
    else if(ch == '\n')
      line_nuber++;
  }

  void CheckDelimiter()
  {
    switch(ch)
    {
    case L'/':
      state = st_expect_close_tag;
      break;
    case L'>':
      CloseTag(false);
      break;
    case L' ':
      state = st_collect_attrib;
      break;
    default:
      state = st_error;
      break;
    }
  }

  void CloseTag(bool closed_tag)
  {
      if (current->Parent != nullptr)
      {
          current->Parent->Child.push_back(current);
          if(closed_tag)
            current = current->Parent;
          state = st_default_state;
      }
      else if (root == nullptr)
      {
          state = st_default_state;
          root = current;
      }
      else
      {
          wcerr << "Document structure error: no root node" << endl;
          state = st_error;
      }
  }

  void CloseFinalTag()
  {
      if (ch != L'>')
          tag += ch;
      else if (current->NodeName != tag)
      {
          wcerr << "Tags not matched" << endl;
          state = st_error;
      }
      else
      {
          current = current->Parent;
          tag.clear();
          state = st_default_state;
      }
  }

public:
  bool Fetch(wchar_t ch)
  {
    this->ch = ch;
    switch(state)
    {
      case st_startup:
        if(ch == 0x00ef)
          state = st_signature_2;
        else if(ch == L'<')
          state = st_check_tag;
        else
          state = st_error;
        break;
      case st_default_state:
        DeafaultState();
        break;
      case st_signature_2:
        if(ch == 0x00bb)
          state = st_signature_3;
        else
          state = st_error;
        break;
      case st_signature_3:
        if(ch == 0x00bf)
          state = st_default_state;
        else
          state = st_error;
        break;

      case st_check_tag:
        Distinguish();
        break;
      case st_close_tag_state:
        CloseFinalTag();
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
        break;
      case st_collect_attribute_value:
          CollectValue();
          break;
      case st_processing_instruction:
        if(ch == L'>')
          state = st_default_state;
        break;
      case st_check_delimiter:
        CheckDelimiter();
        break;
      case st_expect_close_tag:
        if(ch != L'>')
          state = st_error;
        else
          CloseTag(true);
        break;
      default:
        cout << "State not parsed" << endl;
        state = st_error;
        break;
    }
    wcout << ch;
    return state != st_error;
  }

  D3D_Node * GetRoot()
  {
      return state != st_error ? root : nullptr;
  }

  Parser()
  {
    state = st_startup;
    line_nuber = 1;
    root = nullptr;
    current = nullptr;
  }
};

D3D_Node * Load(wistream * stream)
{
    D3D_Node		*   result = nullptr;
    Parser	            parser;
    wchar_t	            ch;

    while (stream->get(ch))
        if (!parser.Fetch(ch))
        {
            cerr << "Input file format error" << endl;
            break;
        }
    return parser.GetRoot();
}

D3D_Node * Load(const char * filename)
{
  D3D_Node		*   result = nullptr;
  wifstream	        report;

  report.open(filename);
  if(!report == true)
    return nullptr;
  result = Load(&report);
  report.close();
  return result;
}

int main(int argc, char * argv[])
{
  if(argc < 2)
  {
    cerr << "Not enough program arguments" << endl;
    return -1;
  }
  D3D_Node * xml_tree = Load(argv[1]);
  return 0;
}
