struct error { uint error; }
contract C {
	error test();
	error _struct;
	function f() public {
		revert test();
	}
	function g(uint x) public returns (uint) {
		_struct.error = x;
		return _struct.error;
	}
}
// ====
// compileToEwasm: also
// ----
// f() -> FAILURE, hex"f8a8fd6d"
// g(uint256): 7 -> 7
