interface I {
	function I() external;
}
// ----
// Warning: (15-37): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (15-37): Constructor must be public or internal.
// TypeError: (15-37): Constructor cannot be defined in interfaces.
// TypeError: (15-37): Constructor must be implemented if declared.
