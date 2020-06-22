==== Source: a ====
struct S { uint x; }
==== Source: b ====
import "a" as A;
struct T { uint x; }
contract C {
    function f() public pure {
        T;
        A.S;
    }
}
// ----
// Warning: (b:90-91): Statement has no effect.
// Warning: (b:101-104): Statement has no effect.
