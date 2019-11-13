pragma experimental SMTChecker;
contract C {
	function(uint) m_g;
    function f(function(uint) internal g) internal {
		g(2);
    }
	function h() public {
		f(m_g);
	}
}
// ----
// Warning: (121-125): Assertion checker does not yet implement this type of function call.
// Warning: (121-125): Assertion checker does not yet implement this type of function call.
