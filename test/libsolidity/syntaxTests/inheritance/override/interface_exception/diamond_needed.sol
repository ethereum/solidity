interface I {
	function f() external;
}
interface J {
	function f() external;
}
contract C is I, J {
	function f() external {}
}
// ----
