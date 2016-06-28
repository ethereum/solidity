#ifndef __FORMAT_H__
#define __FORMAT_H__

#include <libsolidity/ast/AST.h>

using namespace dev::solidity;

std::string navigationMarkdown(ContractDefinition const& contract);
std::string formatMarkdown(ContractDefinition const& contract);

#endif
