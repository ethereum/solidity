contract A { modifier mod(uint a) { _; } }
contract B is A { uint public mod; }
// ----
// DeclarationError 9097: (61-76): Identifier already declared.
// TypeError 1456: (61-76): Override changes modifier to public state variable.
