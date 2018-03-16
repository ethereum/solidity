contract A { function mod(uint a) public { } }
contract B is A { modifier mod(uint a) { _; } }
// ----
// DeclarationError: Identifier already declared.
// TypeError: Override changes function to modifier.
