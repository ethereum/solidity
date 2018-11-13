pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(msg.sig == 0x00000000);
	}
}
// ----
// Warning: (79-108): Assertion violation happens here
