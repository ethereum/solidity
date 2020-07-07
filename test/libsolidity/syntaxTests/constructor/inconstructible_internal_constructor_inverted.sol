// Previously, the type information for A was not yet available at the point of
// "new A".
contract B {
	A a;
	constructor() public {
		a = new A(address(this));
	}
}
contract A {
	constructor(address) internal {}
}
// ----
// TypeError 9054: (141-146): Contract with internal constructor cannot be created directly.
