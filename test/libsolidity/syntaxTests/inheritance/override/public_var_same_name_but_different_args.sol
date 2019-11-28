contract A {
	function foo(uint) internal virtual pure returns(uint) { return 5; }
}
contract X is A {
	uint public foo;
}
// ----
