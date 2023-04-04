==== Source: A.sol ====
function suffix(uint) pure suffix returns (uint) {
    revert();
}

==== Source: B.sol ====
import "A.sol" as A;

contract C {
    function f() public pure {
        1 A.suffix;
        uint a = 0;
        a;
    }
}
// ----
// Warning 5740: (B.sol:94-115): Unreachable code.
