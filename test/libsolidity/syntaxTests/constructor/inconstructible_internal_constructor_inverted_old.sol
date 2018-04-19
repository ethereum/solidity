// Previously, the type information for A was not yet available at the point of
// "new A".
contract B {
	A a;
	function B() public {
		a = new A(this);
	}
}
contract A {
	function A(address a) internal {}
}
// ----
// Warning: (112-155): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (172-205): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (140-145): Contract with internal constructor cannot be created directly.
