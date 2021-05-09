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
