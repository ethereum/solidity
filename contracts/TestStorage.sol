// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

contract Test {
	uint256 slock;

	modifier reentrancyGuard() {
		require(slock == 0, 'Already locked');
		slock = 1;
		_;
		slock = 0;
	}

	function requiresLock() external reentrancyGuard {
		require(true, 'Yay');
	}
}
