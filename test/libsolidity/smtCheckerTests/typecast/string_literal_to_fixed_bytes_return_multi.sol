contract C {
    function h() public pure returns (bytes32 val, bytes3 val2) { return ("abc", "def"); }
    function g() public pure returns (bytes32 val) { return "abc"; }
    function f1() public pure returns (bytes32 val) { return g(); }
    function f2() public pure returns (bytes32 val, bytes3 val2) { return h(); }

	function a() public pure {
		(bytes32 v1, bytes3 v2) = f2();
		assert(v1 == "abc");
		assert(v2 == "cde");
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (410-429): CHC: Assertion violation happens here.\nCounterexample:\n\nv1 = 44048180597813453602326562734351324025098966208897425494240603688123167145984\nv2 = 6579558\n\nTransaction trace:\nC.constructor()\nC.a()\n    C.f2() -- internal call\n        C.h() -- internal call
