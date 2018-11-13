pragma experimental SMTChecker;

contract C
{
	function f() public payable {
		assert(blockhash(2) > 0);
	}
}
// ----
// Warning: (79-103): Assertion violation happens here
