==== Source: A.sol ====
==== Source: B.sol ====
import "A.sol" as A;

contract C {
    uint a = 1000 A;
}
// ----
// TypeError 5704: (B.sol:48-54): Module cannot be used as a literal suffix.
