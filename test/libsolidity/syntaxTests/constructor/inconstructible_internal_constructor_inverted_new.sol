// Previously, the type information for A was not yet available at the point of
// "new A".
contract B {
	A a;
	constructor() public {
		a = new A(this);
	}
}
contract A {
	constructor(address a) internal {}
}
// ----
// TypeError: (141-146): Contract with internal constructor cannot be created directly.
