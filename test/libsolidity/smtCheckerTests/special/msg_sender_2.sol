pragma experimental SMTChecker;

contract C
{
	function f() public pure {
		require(msg.sender != address(0));
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
	}
}
// ----
// Warning: (98-108): Type conversion is not yet fully supported and might yield false positives.
