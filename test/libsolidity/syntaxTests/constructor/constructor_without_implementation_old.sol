contract C {
	function C();
}
// ----
// Warning: (14-27): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (14-27): Constructor must be implemented if declared.
