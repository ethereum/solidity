error E();
contract A {
	uint8[] x;
	function f() public {
		for (uint i = 0; i < 100; ++i)
			x.push(uint8(i));
		revert E();
	}
}
contract B {
	function f() public {
		(new A()).f();
	}
}
// ----
// f() -> FAILURE, hex"92bbf6e8"
// gas irOptimized: 221918
// gas irOptimized code: 42800
// gas legacy: 233752
// gas legacy code: 38000
// gas legacyOptimized: 224863
// gas legacyOptimized code: 34200
