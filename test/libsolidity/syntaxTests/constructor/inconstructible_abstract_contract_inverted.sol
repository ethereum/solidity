// Previously, the type information for A was not yet available at the point of
// "new A".
contract B {
	A a;
	constructor() {
		a = new A(address(this));
	}
}
abstract contract A {
	constructor(address) {}
}
// ----
// TypeError 4614: (134-139): Cannot instantiate an abstract contract.
