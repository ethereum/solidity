pragma abicoder v2;
contract C {
	enum E {X, Y}
	function f(E[] calldata arr) external {
		arr[1];
	}
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// f(uint8[]): 0x20, 2, 3, 3 -> FAILURE
