pragma experimental SMTChecker;
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
// ----
// Warning: (442-461): Assertion violation happens here
