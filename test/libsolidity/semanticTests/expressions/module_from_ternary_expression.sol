==== Source: A ====
contract D {
}
==== Source: B ====
import "A" as M;

contract C {
    function f() public pure returns (bool) {
        bool flag;
        ((flag = true) ? M : M).D;
        return flag;
    }
}
// ----
// f() -> true
