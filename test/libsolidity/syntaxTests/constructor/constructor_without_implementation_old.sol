contract C {
	function C() public;
}
// ----
// Warning: (14-34): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (14-34): Constructor must be implemented if declared.
