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
	function f2(bool a) public pure returns (bool x) {
		x = a;
		string memory message;
		message = "fancy message!";
		require(a, message);
	}
	function f3(bool a) public pure returns (bool x) {
		x = a;
		require(a, "msg");
	}
	function f4(bool a) public pure returns (bool x) {
		x = a;
		string memory message;
		require(a, message);
	}
}
// ====
// compileViaYul: also
// EVMVersion: >=byzantium
// ----
// f(bool): true -> true
// f(bool): false -> FAILURE
// fail() -> FAILURE
// succeed() -> true
// f2(bool): true -> true
// f2(bool): false -> FAILURE, hex"08c379a0", 0x20, 14, "fancy message!"
// f3(bool): true -> true
// f3(bool): false -> FAILURE, hex"08c379a0", 0x20, 3, "msg"
// f4(bool): true -> true
// f4(bool): false -> FAILURE, hex"08c379a0", 0x20, 0
