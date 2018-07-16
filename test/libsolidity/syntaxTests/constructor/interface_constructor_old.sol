interface I {
	function I() public;
}
// ----
// Warning: (15-35): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (15-35): Functions in interfaces must be declared external.
// TypeError: (15-35): Constructor cannot be defined in interfaces.
// TypeError: (15-35): Constructor must be implemented if declared.
