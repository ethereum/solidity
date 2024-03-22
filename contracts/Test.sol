// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

contract Test {
	uint256 transient tlock;

	modifier reentrancyGuard() {
		require(tlock == 0, 'Already locked');
		tlock = 1;
		_;
		tlock = 0;
	}

	function requiresLock() external reentrancyGuard {
		require(true, 'Yay');
	}
}
