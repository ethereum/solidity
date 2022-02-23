contract C {
	constructor() internal {}
}
contract D {
	function f() public { C c = new C(); c; }
}
// ----
// DeclarationError 1845: (14-39): Non-abstract contracts cannot have internal constructors. Remove the "internal" keyword and make the contract abstract to fix this.
