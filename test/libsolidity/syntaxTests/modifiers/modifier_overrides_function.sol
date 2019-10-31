contract A { modifier mod(uint a) { _; } }
contract B is A { function mod(uint a) public { } }
// ----
// DeclarationError: (61-92): Identifier already declared.
// TypeError: (61-92): Override changes modifier to function.
