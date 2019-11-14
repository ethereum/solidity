pragma experimental SMTChecker;
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
// ----
// Warning: (195-200): Assertion checker does not yet implement this type of function call.
// Warning: (224-269): Assertion checker does not yet support the type of this variable.
// Warning: (284-286): Assertion checker does not yet implement type function (function (uint256))
// Warning: (287-288): Assertion checker does not yet support this global variable.
// Warning: (284-291): Assertion checker does not yet implement this type of function call.
// Warning: (327-329): Internal error: Expression undefined for SMT solver.
// Warning: (327-329): Assertion checker does not yet implement type function (function (uint256))
// Warning: (284-286): Assertion checker does not yet implement type function (function (uint256))
// Warning: (287-288): Assertion checker does not yet support this global variable.
// Warning: (284-291): Assertion checker does not yet implement this type of function call.
