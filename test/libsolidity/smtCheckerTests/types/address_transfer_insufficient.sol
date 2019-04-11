pragma experimental SMTChecker;

contract C
{
	function f(address payable a, address payable b) public {
		require(a.balance == 0);
		a.transfer(600);
		b.transfer(1000);
		// Fails since a == this is possible.
		assert(a.balance == 600);
	}
}
// ----
// Warning: (134-149): Insufficient funds happens here
// Warning: (134-149): Assertion checker does not yet implement this type.
// Warning: (153-169): Insufficient funds happens here
// Warning: (153-169): Assertion checker does not yet implement this type.
// Warning: (213-237): Assertion violation happens here
