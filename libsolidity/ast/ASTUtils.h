// SPDX-License-Identifier: GPL-3.0

#pragma once

namespace solidity::frontend
{

class VariableDeclaration;

/// Find the topmost referenced constant variable declaration when the given variable
/// declaration value is an identifier. Works only for constant variable declarations.
/// Returns nullptr if an identifier in the chain is not referencing a constant variable declaration.
VariableDeclaration const* rootConstVariableDeclaration(VariableDeclaration const& _varDecl);

}
