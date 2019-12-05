contract A {
	function foo() internal virtual pure returns(uint) { return 5; }
}
contract X is A {
	uint public foo;
}
// ----
// TypeError: (100-115): Public state variables can only override functions with external visibility.
