contract C
{
	enum D { Left, Right }
	function f(D a) public pure {
		assert(a == D.Left || a == D.Right);
	}
}
// ====
// SMTEngine: all
// ----
