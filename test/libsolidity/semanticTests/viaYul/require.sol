contract C {
	function f(bool a) public pure returns (bool x) {
		bool b = a;
		x = b;
		require(b);
	}
	function fail() public pure returns (bool x) {
		x = true;
		require(false);
	}
	function succeed() public pure returns (bool x) {
		x = true;
		require(true);
	}
	/* Not properly supported by test system yet
	function f2(bool a) public pure returns (bool x) {
		x = a;
		string memory message;
		require(a, message);
	}*/
}
// ====
// compileViaYul: true
// ----
// f(bool): true -> true
// f(bool): false -> FAILURE
// fail() -> FAILURE
// succeed() -> true
