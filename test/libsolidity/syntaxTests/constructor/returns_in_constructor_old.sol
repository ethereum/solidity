contract test {
	function test() public returns (uint a) { }
}
// ----
// SyntaxError: (17-60): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (48-56): Non-empty "returns" directive for constructor.
