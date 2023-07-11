contract C {
  function f() public pure returns (uint y) {
    unchecked{{
        uint max = type(uint).max;
        uint x = max + 1; // overflow not reported
        y = x;
    }}
	assert(y == 0);
  }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
