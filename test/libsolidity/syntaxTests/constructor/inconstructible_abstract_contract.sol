abstract contract C {
	constructor() {}
}
contract D {
	function f() public { C c = new C(); c; }
}
// ----
// TypeError 4614: (84-89='new C'): Cannot instantiate an abstract contract.
