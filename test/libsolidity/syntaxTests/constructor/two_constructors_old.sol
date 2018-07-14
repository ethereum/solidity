contract test {
	function test(uint a) public { }
	function test() public {}
}
// ----
// SyntaxError: (17-49): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// SyntaxError: (51-76): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// DeclarationError: (51-76): More than one constructor defined.
