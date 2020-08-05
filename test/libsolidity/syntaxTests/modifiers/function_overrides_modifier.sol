contract A { function mod(uint a) public { } }
contract B is A { modifier mod(uint a) { _; } }
// ----
// DeclarationError 9097: (65-92): Identifier already declared.
// TypeError 5631: (65-92): Override changes function or public state variable to modifier.
