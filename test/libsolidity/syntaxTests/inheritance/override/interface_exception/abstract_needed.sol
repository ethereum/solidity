abstract contract I {
	function f() external virtual;
}
contract C is I {
	function f() external {}
}
// ----
