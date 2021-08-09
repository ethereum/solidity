// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C {
	fallback() external {
		assembly {
			let x := calldataload(0)
			x := x
			sstore(0, x)
		}
	}
}
