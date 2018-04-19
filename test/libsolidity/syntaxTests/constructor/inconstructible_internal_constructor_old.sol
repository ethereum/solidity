contract C {
	function C() internal {}
}
contract D {
	function f() public { C x = new C(); x; }
}
// ----
// Warning: (14-38): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (83-88): Contract with internal constructor cannot be created directly.
