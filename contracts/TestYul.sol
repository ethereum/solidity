// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

contract Test {
	modifier reentrancyGuard {
		uint256 tlock;
		assembly {
			tlock := tload(0)
		}
		require(tlock == 0, 'Already locked');
		assembly {
			tstore(0, 1)
		}
		_;
		assembly {
			tstore(0, 0)
		}
	}

	function requiresLock() external reentrancyGuard {
		require(true, 'Yay');
	}
}
