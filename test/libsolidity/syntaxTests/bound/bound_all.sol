library L {
	function g(function() internal returns (uint) _t) internal returns (uint) { return _t(); }
}
contract C {
	using L for *;
	function f() public returns (uint) {
		return t.g();
	}
	function t() public pure returns (uint)  { return 7; }
}
