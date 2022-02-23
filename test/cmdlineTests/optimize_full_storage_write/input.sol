// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract OptimizeFullSlotWrite {
	uint64[4] nums;
	function f() public {
		nums[0] = 11111;
		nums[1] = 22222;
		nums[2] = 33333;
		nums[3] = 44444;
	}
}
