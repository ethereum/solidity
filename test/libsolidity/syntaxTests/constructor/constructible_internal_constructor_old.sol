contract C {
	function C() internal {}
}
contract D is C {
	function D() public {}
}
// ----
// SyntaxError: (14-38): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// SyntaxError: (60-82): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
