pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(msg.data.length > 0);
	}
}
// ----
// Warning 6328: (79-106): CHC: Assertion violation happens here.
