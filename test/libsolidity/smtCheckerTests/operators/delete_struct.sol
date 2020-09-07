pragma experimental SMTChecker;

contract C
{
	struct S
	{
		uint x;
	}
	function f(bool b) public pure {
		S memory s;
		s.x = 2;
		if (b)
			delete s;
		else
			delete s.x;
		assert(s.x == 0);
	}
}
// ----
