contract A {
	uint i;
}
contract B is A {
	uint i;
}
// ----
// DeclarationError: (43-49): Identifier already declared.
