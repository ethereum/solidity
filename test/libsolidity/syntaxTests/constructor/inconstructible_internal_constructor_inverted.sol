// Previously, the type information for A was not yet available at the point of
// "new A".
contract B {
	A a;
	constructor() public {
		a = new A(address(this));
	}
}
contract A {
	constructor(address) public {}
}
// ----
