pragma experimental SMTChecker;
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
// ----
// Warning: (123-128): Assertion checker does not yet implement this type of function call.
// Warning: (152-197): Assertion checker does not yet support the type of this variable.
// Warning: (212-214): Assertion checker does not yet implement type function (function (uint256))
// Warning: (212-219): Assertion checker does not yet implement this type of function call.
// Warning: (255-257): Internal error: Expression undefined for SMT solver.
// Warning: (255-257): Assertion checker does not yet implement type function (function (uint256))
// Warning: (212-214): Assertion checker does not yet implement type function (function (uint256))
// Warning: (212-219): Assertion checker does not yet implement this type of function call.
