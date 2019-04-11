pragma experimental SMTChecker;

contract C
{
	function f(uint x, address payable a, address payable b) public {
		require(a != b);
		require(x == 100);
		require(x == a.balance);
		require(a.balance == b.balance);
		a.transfer(600);
		b.transfer(100);
		// Fails since a == this is possible.
		assert(a.balance > b.balance);
	}
}
// ----
// Warning: (217-232): Insufficient funds happens here
// Warning: (217-232): Assertion checker does not yet implement this type.
// Warning: (236-251): Insufficient funds happens here
// Warning: (236-251): Assertion checker does not yet implement this type.
// Warning: (295-324): Assertion violation happens here
