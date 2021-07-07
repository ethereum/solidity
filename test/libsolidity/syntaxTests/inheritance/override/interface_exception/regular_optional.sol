interface I {
	function f() external;
	function g() external;
	function h() external;
}
contract C is I {
	function f() external {}
	function g() external override {}
	function h() external override(I) {}
}
// ----
