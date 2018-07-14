contract C {
	function C() public;
}
// ----
// SyntaxError: (14-34): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (14-34): Constructor must be implemented if declared.
