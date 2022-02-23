contract C {
	function(uint) m_g;
    function f(function(uint) internal g) internal {
		g(2);
    }
	function h() public {
		f(m_g);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 5729: (89-93): BMC does not yet implement this type of function call.
