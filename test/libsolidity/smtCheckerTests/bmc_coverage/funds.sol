contract C {
	function f(address payable a) public {
		a.transfer(200);
	}
}
// ====
// SMTEngine: bmc
// ----
// Warning 1236: (55-70): BMC: Insufficient funds happens here.
