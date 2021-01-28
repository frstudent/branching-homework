#include <string>
#include <list>

class D3D_Attrib
{
public:
    std::wstring Name;
    std::wstring Value;
    D3D_Attrib(std::wstring name, std::wstring value)
    {
        Name = name;
        Value = value;
    }
};

class D3D_Node
{
public:
    std::wstring                NodeName;
    D3D_Node                *   Parent;
    std::list<D3D_Attrib *>     Attributes;
    std::list<D3D_Node *>       Child;

    D3D_Node(std::wstring name, D3D_Node * parent)
    {
        NodeName = name;
        Parent = parent;
    }
};

