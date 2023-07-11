==== Source: A.sol ====
==== Source: B.sol ====
import "A.sol" as A;

contract C {
    uint a = A(1000);
}
// ----
// TypeError 5704: (B.sol:48-55): This expression is not callable.
