==== Source: s1.sol ====
import "s1.sol" as A;

library L {
    function f() internal pure {}
}

contract C
{
    function test() public pure {
        A.L;
    }
}
// ----
// Warning 6133: (s1.sol:127-130='A.L'): Statement has no effect.
