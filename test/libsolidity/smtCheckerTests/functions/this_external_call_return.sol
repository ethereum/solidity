pragma experimental SMTChecker;

contract C
{
	uint x;
	function f(uint y) public returns (uint) {
		x = y;
		return x;
	}
	function g(uint y) public {
		require(y < 1000);
		uint z = this.f(y);
		assert(z < 1000);
	}
}
