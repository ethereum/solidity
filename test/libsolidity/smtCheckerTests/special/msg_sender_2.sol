contract C
{
	function f() public view {
		require(msg.sender != address(0));
		address a = msg.sender;
		address b = msg.sender;
		assert(a == b);
	}
}
// ====
// SMTEngine: all
// ----
