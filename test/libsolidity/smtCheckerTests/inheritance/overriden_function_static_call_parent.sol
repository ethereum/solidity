contract BaseBase {
	uint x;
	function init(uint a, uint b) public virtual {
		x = a;
	}
}
contract Base is BaseBase {
	function init(uint a, uint b) public override {
	}
}
contract Child is Base {
	function bInit(uint c, uint d) public {
		BaseBase.init(c, d);
		assert(x == c);
		assert(x == d); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 5667: (52-58): Unused function parameter. Remove or comment out the variable name to silence this warning.
// Warning 6328: (282-296): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
