contract C {
	constructor() internal {}
}
contract D {
	function f() public { C c = new C(); c; }
}
// ----
// TypeError: (84-89): Contract with internal constructor cannot be created directly.
