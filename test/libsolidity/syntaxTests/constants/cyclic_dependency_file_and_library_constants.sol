==== Source: A.sol ====
import "B.sol";

uint256 constant A = B.VAL + 1;

==== Source: B.sol ====
import "A.sol";

library B {
    uint256 constant VAL = A + 1;
}

// ----
// TypeError 6161: (B.sol:33-61): The value of the constant VAL has a cyclic dependency via A.
// TypeError 6161: (A.sol:17-47): The value of the constant A has a cyclic dependency via VAL.
