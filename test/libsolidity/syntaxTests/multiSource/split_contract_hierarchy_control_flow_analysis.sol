==== Source: s1.sol ====
contract C
{
	function normal() public pure returns (uint) { return 1337; }
	function reverting() public pure returns (uint) { revert(); }
}
==== Source: s2.sol ====
import "s1.sol";
contract D is C
{
	function foo() public pure returns (uint) { normal(); }
	function bar() public pure returns (uint) { reverting(); }
}
// ----
// Warning 6321: (s2.sol:72-76): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
