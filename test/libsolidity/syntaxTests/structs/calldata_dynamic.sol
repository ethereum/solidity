pragma abicoder               v2;
contract Test {
	struct S { int[] a; }
	function f(S calldata) external { }
}
// ----
