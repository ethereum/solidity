contract test {
	function test(uint) public { }
	constructor() public {}
}
// ----
// Warning: (17-47): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// DeclarationError: (49-72): More than one constructor defined.
