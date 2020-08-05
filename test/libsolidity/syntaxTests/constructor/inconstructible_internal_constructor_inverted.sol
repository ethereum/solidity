// Previously, the type information for A was not yet available at the point of
// "new A".
contract B {
	A a;
	constructor() {
		a = new A(address(this));
	}
}
contract A {
	constructor(address) internal {}
}
// ----
// DeclarationError 1845: (175-207): Non-abstract contracts cannot have internal constructors. Remove the "internal" keyword and make the contract abstract to fix this.
