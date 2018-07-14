contract C {
	function C() internal {}
}
contract D {
	function f() public { C x = new C(); x; }
}
// ----
// SyntaxError: (14-38): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (83-88): Contract with internal constructor cannot be created directly.
