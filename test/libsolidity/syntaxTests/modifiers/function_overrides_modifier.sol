contract A { function mod(uint a) public { } }
contract B is A { modifier mod(uint a) { _; } }
// ----
// DeclarationError: (65-92): Identifier already declared.
// TypeError: (65-92): Override changes function to modifier.
