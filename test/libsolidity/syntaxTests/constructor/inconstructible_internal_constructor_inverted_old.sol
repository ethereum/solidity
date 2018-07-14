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
// SyntaxError: (112-155): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// SyntaxError: (172-205): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (140-145): Contract with internal constructor cannot be created directly.
