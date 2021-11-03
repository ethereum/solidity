contract A1 {}
contract B1 is A1 { constructor() payable {} }

contract A2 { constructor() {} }
contract B2 is A2 { constructor() payable {} }

contract B3 { constructor() payable {} }

contract C {
	function f() public payable returns (bool) {
		// Make sure none of these revert.
		new B1{value: 10}();
		new B2{value: 10}();
		new B3{value: 10}();
		return true;
	}
}
// ====
// compileViaYul: also
// ----
// f(), 2000 ether -> true
// gas irOptimized: 123037
// gas legacy: 123226
// gas legacyOptimized: 123092
