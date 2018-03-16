contract A { modifier mod(uint a) { _; } }
contract B is A { function mod(uint a) public { } }
// ----
// DeclarationError: Identifier already declared.
// TypeError: Override changes modifier to function.
