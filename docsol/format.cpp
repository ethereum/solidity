#include "format.h" 
#include <sstream>
#include <boost/algorithm/string.hpp>

string extractDoc(
        multimap<string, DocTag> const& _tags,
        string const& _name
) {
    string value;
    auto range = _tags.equal_range(_name);
    for (auto i = range.first; i != range.second; ++i)
        value += i->second.content;
    return value;
}

map<string,string> extractDocMap(
        multimap<string, DocTag> const& _tags,
        string const& _name
) {
    map<string,string> value;
    auto range = _tags.equal_range(_name);
    for (auto i = range.first; i != range.second; ++i)
        value[i->second.paramName] = i->second.content;
    return value;
}


string navigationMarkdown(ContractDefinition const& contract)
{
    stringstream md;

    string lower_name = contract.name();
    boost::algorithm::to_lower(lower_name); 

    md << "* [" << contract.name() << "](#"
       << lower_name << (contract.isLibrary() ? "-library" : "")
       << ")" << endl;
    
    return md.str();
}

void formatMethod(stringstream &md, FunctionDefinition const& method)
{
    auto doc          = method.annotation().docTags; 
    auto param_vector = method.parameterList().parameters();
    auto return_vector= method.returnParameterList()->parameters();
    auto desc         = extractDoc(doc, "dev");
    auto notice       = extractDoc(doc, "notice");
    auto returns      = extractDoc(doc, "return");
    auto param_doc    = extractDocMap(doc, "param");

    md << "```js" << endl << method.name() << "(";

    for (auto param: param_vector)
        md << param->name() << (param == param_vector.back() ? "" : ", ");

    if (return_vector.size() > 0)
    {
        md << ") returns (";

        for (auto param: return_vector)
            md << param->type()->canonicalName(false)
               << (param == return_vector.back() ? "" : ", ");
    }

    md << ")" << endl << "```" << endl
       << desc << endl;

    if (!notice.empty())
        md << endl << "**Notice:**" << notice << endl << endl;
 
    if (param_vector.size() > 0)
    {
        md << "##### Parameters" << endl << endl;

        for (auto param: param_vector)
            md << "1. `" << param->type()->canonicalName(false)
               << " " << param->name() << "` "
               << param_doc[param->name()] << endl;
        
        md << endl;
    }
 
    if (return_vector.size() > 0)
    {
        md << "##### Returns" << endl << endl;

        for (auto param: return_vector)
            md << "1. `" << param->type()->canonicalName(false)
               << " " << param->name() << "` - "
               << returns << endl;
 
        md << endl;
    }
}

string formatMarkdown(ContractDefinition const& contract)
{
    stringstream md;
    auto doc   = contract.annotation().docTags; 
    auto title = extractDoc(doc, "title");
    auto desc  = extractDoc(doc, "dev");
 
    md << "### " << contract.name()
       << (contract.isLibrary() ? " `library`" : "") << endl
       << title << endl << endl << desc << endl;

    for (auto fun: contract.definedFunctions())
    {
        md  << "#### " << (fun->isConstructor() ? "Constructor" : fun->name()) << endl;
        formatMethod(md, *fun);
    }

    md << endl << "---" << endl;
    return md.str();
}
