contract A {
	uint public x;
}
contract C is A {
	function x() public returns (uint);
}
// ----
// DeclarationError: (50-85): Identifier already declared.
