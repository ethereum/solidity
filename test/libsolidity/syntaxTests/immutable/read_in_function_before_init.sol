abstract contract A {
	uint public t;
	constructor() { t = f(); }

	function f() virtual pure internal returns (uint);
}
contract B is A {
	uint immutable x = 2;
	function f() override pure internal returns (uint) { return x; }
}
