pragma experimental SMTChecker;
contract C {
	function f(address payable a) public {
		a.transfer(200);
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 1236: (87-102): BMC: Insufficient funds happens here.
