contract A {
	uint public x;
}
contract C is A {
	function x() public returns (uint) {}
}
// ----
// DeclarationError: (50-87): Identifier already declared.
