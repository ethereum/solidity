contract test {
	function test() public returns (uint a) { }
}
// ----
// Warning: (17-60): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (48-56): Non-empty "returns" directive for constructor.
