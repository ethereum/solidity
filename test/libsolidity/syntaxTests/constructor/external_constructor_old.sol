contract test {
	function test() external {}
}
// ----
// Warning: (17-44): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (17-44): Constructor must be public or internal.
