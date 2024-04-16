contract C {
	function bitwiseXor(int[10] memory p) public pure {
        1 ^ p[0];
	}

	function bitwiseAnd(int[10] memory p) public pure {
		1 & p[0];
	}

	function bitwiseOr(int[10] memory p) public pure {
		1 | p[0];
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
