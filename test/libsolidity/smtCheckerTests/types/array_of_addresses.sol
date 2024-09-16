// SPDX-License-Identifier: GPL-3.0

pragma solidity >=0.0.0;
contract C {
	address[10] a;
	address payable[10] b;

	function f() public {
		a = b;
		assert(a[0] == b[0]);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
