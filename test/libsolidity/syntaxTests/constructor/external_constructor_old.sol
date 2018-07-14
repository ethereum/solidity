contract test {
	function test() external {}
}
// ----
// SyntaxError: (17-44): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (17-44): Constructor must be public or internal.
