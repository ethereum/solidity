contract C {
	function f() public {
		function() internal returns (uint) _f;
		_f();
	}
	function g() public {
		function() external returns (uint) _g;
		_g();
	}
	function h1() internal returns (function() internal returns (uint) _h) {}
	function h2() public {
		h1()();
	}
	function k1() internal returns (function() external returns (uint) _k) {}
	function k2() public {
		k1()();
	}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> FAILURE, hex"4e487b71", 0x51
// g() -> FAILURE
// h2() -> FAILURE, hex"4e487b71", 0x51
// k2() -> FAILURE
