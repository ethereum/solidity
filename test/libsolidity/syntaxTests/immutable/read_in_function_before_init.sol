abstract contract A {
	uint public t;
	constructor() { t = f(); }

	function f() virtual view internal returns (uint);
}
contract B is A {
	uint immutable x = 2;
	function f() override view internal returns (uint) { return x; }
}
// ----
// TypeError 7733: (223-224): Immutable variables cannot be read before they are initialized.
