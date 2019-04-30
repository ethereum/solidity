pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		uint a = 1;
		uint b = 3;
		uint c = 7;
		a += b += c;
		assert(b ==  10 && a == 11);
		a += (b += c);
		assert(b ==  17 && a == 28);
		a += a += a;
		assert(a == 112);
	}
}
