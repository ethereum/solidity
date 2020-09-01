==== Source: a ====
function f(uint x) pure returns (uint) { return x * 3; }
==== Source: b ====
import "a" as A;
function g(uint x) pure returns (uint) { return A.f(x) * 3; }
==== Source: c ====
import "b" as B;
contract C {
    function f() public pure {
        B.g(2);
        B.A.f(3);
    }
}
// ----
