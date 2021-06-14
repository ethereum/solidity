contract C
{
	enum D { Left, Right }
	D d;
	function f(D _a) public {
		require(_a == D.Left);
		d = D.Right;
		assert(d != _a);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 0: (0-133): Contract invariants for :C:\n((d = 0) || (d = 1))\n
