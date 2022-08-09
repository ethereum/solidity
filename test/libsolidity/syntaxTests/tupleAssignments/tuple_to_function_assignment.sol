contract C {
	function f() internal pure {}
	function g() internal pure returns (uint256) {}
	function h() internal pure returns (uint256, uint256) {}
	function test() public pure {
		f() = ();
		g() = (uint256(1));
		h() = (uint256(1), uint256(2));
		h() = ();
	}
}
// ----
// TypeError 4247: (184-187): Expression has to be an lvalue.
// TypeError 4247: (196-199): Expression has to be an lvalue.
// TypeError 4247: (218-221): Expression has to be an lvalue.
// TypeError 4247: (252-255): Expression has to be an lvalue.
// TypeError 7407: (258-260): Type tuple() is not implicitly convertible to expected type tuple(uint256,uint256).
