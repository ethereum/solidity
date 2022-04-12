==== Source: A.sol ====
import "B.sol";

uint256 constant A = B + 1;

==== Source: B.sol ====
import "A.sol";

uint256 constant B = A + 1;

// ----
// TypeError 6161: (B.sol:17-43): The value of the constant B has a cyclic dependency via A.
// TypeError 6161: (A.sol:17-43): The value of the constant A has a cyclic dependency via B.
