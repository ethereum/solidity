contract A {
	function g() public { revert("fail"); }
}

contract C {
	A a = new A();
	function f() public {
		a.g();
	}
}
// ====
// revertStrings: debug
// ----
// f() -> FAILURE, hex"08c379a0", 0x20, 4, "fail"
