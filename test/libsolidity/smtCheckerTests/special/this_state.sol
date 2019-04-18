pragma experimental SMTChecker;

contract C
{
	address thisAddr;
	function f(address a) public {
		require(a == address(this));
		thisAddr = a;
		assert(thisAddr == address(this));
	}
}
