// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
	function f(uint x, uint y, uint k) public pure {
		require(k > 0);
		require(x % k == 0);
		require(y % k == 0);
		uint r = mulmod(x, y, k);
		assert(r % k == 0);
	}
}