// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
pragma abicoder v2;

contract Arraysum {
	uint256[] values;

	function sumArray() public view returns(uint sum) {
		sum = 0;
		// The optimizer should read the length of the array only once, because
		// LoopInvariantCodeMotion can move the `sload` corresponding to the length outside of the
		// loop.
		for(uint i = 0; i < values.length; i++)
			sum += values[i];
	}
}
