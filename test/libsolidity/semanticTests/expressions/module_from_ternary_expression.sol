==== Source: A ====
contract C {
}
==== Source: B ====
import "A" as M;

contract C {
    function f() public pure returns (bool) {
        bool flag;
        ((flag = true) ? M : M).C;
        return flag;
    }
}
// ----
// f() -> true
