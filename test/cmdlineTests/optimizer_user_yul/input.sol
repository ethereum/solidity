// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;

contract C
{
	constructor() public payable
	{
		int a;

		// Can't be optimized due to external reference "a"
		assembly
		{
			let x,y,z

			sstore(0, 1)

			for { } sload(4) { } {
				z := exp(x, y)
			}

			a := 2
		}

		// Can be optimized due to no external references
		assembly
		{
			let x,y,z

			sstore(2, 3)

			for { } sload(5) { } {
				// Expected to be optimized out for yulOptimizer, but not for
				// old optimizer
				z := exp(x, y)
			}
		}
	}
}
