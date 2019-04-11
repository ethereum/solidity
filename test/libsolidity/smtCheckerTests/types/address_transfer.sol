pragma experimental SMTChecker;

contract C
{
	function f(address payable a) public {
		uint x = 100;
		require(x == a.balance);
		a.transfer(600);
		// This fails since a == this is possible.
		assert(a.balance == 700);
	}
}
// ----
// Warning: (131-146): Insufficient funds happens here
// Warning: (131-146): Assertion checker does not yet implement this type.
// Warning: (195-219): Assertion violation happens here
