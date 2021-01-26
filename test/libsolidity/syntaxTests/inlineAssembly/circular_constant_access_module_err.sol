==== Source: a ====
import "b";
uint constant c = d;
==== Source: b ====
import "a" as M;
uint constant b = M.c;
uint constant d = b;
contract C {
    uint constant a = b;
    function f() public returns (uint t) {
        assembly {
            t := a
        }
    }
}
// ----
// TypeError 3558: (b:178-179): Constant variable is circular.
