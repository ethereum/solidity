contract test {
	function test(uint a) public { }
	function test() public {}
}
// ----
// Warning: (17-49): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (51-76): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// DeclarationError: (51-76): More than one constructor defined.
