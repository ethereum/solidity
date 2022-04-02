contract A { function Err() public pure {} }
contract B { error Err(); }
contract C is A, B {}
// ----
// DeclarationError 9097: (58-70='error Err();'): Identifier already declared.
