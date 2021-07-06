==== Source: s1.sol ====
contract C
{
	function normal(bool x) public pure returns (uint)
	{
		if (x)
			return xxx();
		else
			return yyy();
	}
	function yyy() public pure returns (uint) { revert(); }
	function bar() public pure returns (uint) { normal(true); }

	function xxx() public virtual pure returns (uint) { return 1; }
}
==== Source: s2.sol ====
import "s1.sol";
contract D is C
{
	function foo() public pure returns (uint) { normal(false); }
	function xxx() public override pure returns(uint) { revert(); }
}
// ----
// Warning 6321: (s1.sol:215-219): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
