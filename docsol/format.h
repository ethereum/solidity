#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <libsolidity/ast/AST.h>

using namespace std;
using namespace dev::solidity;

string navigationMarkdown(ContractDefinition const& contract);
string formatMarkdown(ContractDefinition const& contract);

#endif
