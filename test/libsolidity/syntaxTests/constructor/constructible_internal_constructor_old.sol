contract C {
	function C() internal {}
}
contract D is C {
	function D() public {}
}
// ----
// Warning: (14-38): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (60-82): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
