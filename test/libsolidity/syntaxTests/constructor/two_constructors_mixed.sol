contract test {
	function test(uint) public { }
	constructor() public {}
}
// ----
// SyntaxError: (17-47): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// DeclarationError: (49-72): More than one constructor defined.
