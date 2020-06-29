// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract Arraysum {
	uint256[] values;

	function sumArray() public view returns(uint) {
		uint sum = 0;

		for(uint i = 0; i < values.length; i++)
			sum += values[i];
	}
}
