contract A { modifier mod(uint a) { _; } }
contract B is A { function mod(uint a) public { } }
// ----
// DeclarationError 9097: (61-92): Identifier already declared.
// TypeError 1469: (61-92): Override changes modifier to function.
