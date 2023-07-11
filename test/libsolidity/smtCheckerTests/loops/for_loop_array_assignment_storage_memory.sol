// Most of the code has been commented out because of nondeterminism in Spacer in Z3 4.8.9
contract LoopFor2 {
	uint[] b;
	//uint[] c;
	function p() public {
		b.push();
	}
	function testUnboundedForLoop(uint n) public {
		require(b.length > 0);
		b[0] = 900;
		//uint[] memory a = b;
		require(n > 0 && n < 100);
		for (uint i = 0; i < n; i += 1) {
			//b[i] = i + 1;
			//c[i] = b[i];
		}
		// This is safe but too hard to solve currently.
		//assert(b[0] == c[0]);
		//assert(a[0] == 900);
		//assert(b[0] == 900);
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
