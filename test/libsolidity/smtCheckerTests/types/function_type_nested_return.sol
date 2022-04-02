contract C {
	function(uint) m_g;
	function r() internal view returns (function(uint)) {
		return m_g;
	}
    function f1(function(uint) internal g1) internal {
		g1(2);
    }
    function f2(function(function(uint) internal) internal g2) internal {
		g2(r());
    }
	function h() public {
		f2(f1);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8115: (192-237): Assertion checker does not yet support the type of this variable.
// Warning 8364: (252-254='g2'): Assertion checker does not yet implement type function (function (uint256))
// Warning 1695: (255-256='r'): Assertion checker does not yet support this global variable.
// Warning 6031: (295-297='f1'): Internal error: Expression undefined for SMT solver.
// Warning 8364: (295-297='f1'): Assertion checker does not yet implement type function (function (uint256))
// Warning 5729: (163-168='g1(2)'): BMC does not yet implement this type of function call.
// Warning 5729: (252-259='g2(r())'): BMC does not yet implement this type of function call.
