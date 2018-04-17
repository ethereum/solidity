interface I {
	function I();
}
// ----
// Warning: (15-28): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (15-28): Functions in interfaces should be declared external.
// TypeError: (15-28): Constructor cannot be defined in interfaces.
// TypeError: (15-28): Constructor must be implemented if declared.
