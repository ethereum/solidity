#include "format.h" 
#include <sstream>
#include <boost/algorithm/string.hpp>

std::string extractDoc(std::multimap<std::string, DocTag> const& _tags,
                       std::string const& _name)
{
    std::string value;
	auto range = _tags.equal_range(_name);
	for (auto i = range.first; i != range.second; ++i)
		value += i->second.content;
	return value;
}

std::map<std::string,std::string>
extractDocMap(std::multimap<std::string, DocTag> const& _tags,
              std::string const& _name)
{
    std::map<std::string,std::string> value;
	auto range = _tags.equal_range(_name);
	for (auto i = range.first; i != range.second; ++i)
		value[i->second.paramName] = i->second.content;
    return value;
}


std::string navigationMarkdown(ContractDefinition const& contract) {
    std::stringstream md;

    std::string lower_name = contract.name();
    boost::algorithm::to_lower(lower_name); 

    md << "* [" << contract.name() << "](#"
       << lower_name << (contract.isLibrary() ? "-library" : "")
       << ")" << std::endl;
    
    return md.str();
}

void formatMethod(std::stringstream &md, FunctionDefinition const& method) {
    auto doc          = method.annotation().docTags; 
    auto param_vector = method.parameterList().parameters();
    auto return_vector= method.returnParameterList()->parameters();
    auto desc         = extractDoc(doc, "dev");
    auto notice       = extractDoc(doc, "notice");
    auto returns      = extractDoc(doc, "return");
    auto param_doc    = extractDocMap(doc, "param");

    md << "```js" << std::endl << method.name() << "(";

    for (auto param: param_vector)
        md << param->name() << (param == param_vector.back() ? "" : ", ");

    if (return_vector.size() > 0) {
        md << ") returns (";

        for (auto param: return_vector)
            md << param->type()->canonicalName(false)
               << (param == return_vector.back() ? "" : ", ");
    }

    md << ")" << std::endl << "```" << std::endl
       << desc << std::endl;

    if (!notice.empty())
        md << std::endl << "**Notice:**" << notice << std::endl << std::endl;
 
    if (param_vector.size() > 0) {
        md << "##### Parameters" << std::endl << std::endl;

        for (auto param: param_vector) {
            md << "1. `" << param->type()->canonicalName(false)
               << " " << param->name() << "` "
               << param_doc[param->name()] << std::endl;
        }
        
        md << std::endl;
    }
 
    if (return_vector.size() > 0) {
        md << "##### Returns" << std::endl << std::endl;

        for (auto param: return_vector) {
            md << "1. `" << param->type()->canonicalName(false)
               << " " << param->name() << "` - "
               << returns << std::endl;
        }
 
        md << std::endl;
    }
}

std::string formatMarkdown(ContractDefinition const& contract) {
    std::stringstream md;
    auto doc   = contract.annotation().docTags; 
    auto title = extractDoc(doc, "title");
    auto desc  = extractDoc(doc, "dev");
 
    md << "### " << contract.name()
       << (contract.isLibrary() ? " `library`" : "") << std::endl
       << title << std::endl << std::endl << desc << std::endl;

    for (auto fun: contract.definedFunctions()) {
        md  << "#### " << (fun->isConstructor() ? "Constructor" : fun->name()) << std::endl;
        formatMethod(md, *fun);
    }

    md << std::endl << "---" << std::endl;
    return md.str();
}
