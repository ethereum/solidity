contract A { function Err() public pure {} }
contract B is A { error Err(); }
// ----
// DeclarationError 9097: (63-75): Identifier already declared.
