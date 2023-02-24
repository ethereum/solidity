==== Source: A.sol ====
function value(uint) pure suffix returns (uint) {}
==== Source: B.sol ====
import "A.sol" as A;

contract C {
    uint x = 1000 A.value;
}
