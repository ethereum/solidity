contract C {
	function(uint) m_g;
    function f1(function(uint) internal g1) internal {
		g1(2);
    }
    function f2(function(function(uint) internal) internal g2) internal {
		g2(m_g);
    }
	function h() public {
		f2(f1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8115: (120-165): Assertion checker does not yet support the type of this variable.
// Warning 8364: (180-182='g2'): Assertion checker does not yet implement type function (function (uint256))
// Warning 6031: (223-225='f1'): Internal error: Expression undefined for SMT solver.
// Warning 8364: (223-225='f1'): Assertion checker does not yet implement type function (function (uint256))
// Warning 5729: (91-96='g1(2)'): BMC does not yet implement this type of function call.
// Warning 5729: (180-187='g2(m_g)'): BMC does not yet implement this type of function call.
