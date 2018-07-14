interface I {
	function I() public;
}
// ----
// SyntaxError: (15-35): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (15-35): Functions in interfaces must be declared external.
// TypeError: (15-35): Constructor cannot be defined in interfaces.
// TypeError: (15-35): Constructor must be implemented if declared.
