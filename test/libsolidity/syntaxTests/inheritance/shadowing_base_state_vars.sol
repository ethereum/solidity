contract A {
	uint i;
}
contract B is A {
	uint i;
}
// ----
// DeclarationError 9097: (43-49): Identifier already declared.
