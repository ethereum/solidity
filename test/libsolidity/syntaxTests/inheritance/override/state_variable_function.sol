contract A {
	uint public x;
}
contract C is A {
	function x() public returns (uint) {}
}
// ----
// DeclarationError: (50-87): Identifier already declared.
// TypeError: (50-87): Overriding function is missing "override" specifier.
// TypeError: (14-27): Cannot override public state variable.
