contract A {
	function foo() internal virtual view returns(uint) { return 5; }
}
contract X is A {
	uint public foo;
}
// ----
// TypeError 9456: (100-115): Overriding public state variable is missing "override" specifier.
// TypeError 5225: (100-115): Public state variables can only override functions with external visibility.
