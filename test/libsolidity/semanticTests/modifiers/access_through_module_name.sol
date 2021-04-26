==== Source: a ====
import "a" as M;
contract C {
    uint public x;
    modifier m { x = 1; _; }

    function f() public M.M.C.m returns (uint t, uint r) {
        t = x;
        x = 3;
        r = 9;
    }
    function g() public m returns (uint t, uint r) {
        t = x;
        x = 4;
        r = 10;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// x() -> 0x00
// f() -> 1, 9
// x() -> 3
// g() -> 1, 0x0a
// x() -> 4
// f() -> 1, 9
// x() -> 3
