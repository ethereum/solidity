==== Source: a ====
bytes32 constant x = x;
==== Source: b ====
import "a";
contract C {
    function f() public pure returns (uint t) {
        assembly {
            // Reference to a circular member
            t := x
        }
    }
}
// ----
// TypeError 3558: (b:155-156): Constant variable is circular.
