==== Source: A ====
function f() pure returns (uint) {
    return 42;
}
==== Source: B ====
function f() pure returns (uint) {
    return 24;
}
==== Source: C ====
import "A" as A;
import "B" as B;

contract C {
    function f(bool b) public pure returns (uint) {
        return (b ? A : B).f();
    }
}
// ----
// TypeError 1080: (C:116-125): True expression's type module "A" does not match false expression's type module "B".
