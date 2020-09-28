pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(msg.sig == 0x00000000);
	}
}
// ----
// Warning 6328: (79-108): CHC: Assertion violation happens here.
