pragma experimental SMTChecker;

contract C
{
	modifier m {
		if (true)
			_;
	}

	function f(address a) m public pure {
		if (true) {
			a = g();
			assert(a == address(0));
		}
	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ----
