contract A {
	int public testvar;
	function foo() internal override(N, Z) returns (uint256);
}
// ----
// DeclarationError: (68-69): Identifier not found or not unique.
