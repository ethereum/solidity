==== Source: a.sol ====
library L {
    type Int is int128;

    function add(Int, Int) pure public returns (Int) {
        return Int.wrap(7);
    }
}
==== Source: b.sol ====
import "a.sol" as a;
==== Source: c.sol ====
import "b.sol" as b;

contract C {
    using {b.a.L.add as +} for b.a.L.Int;

    function f() pure public returns (b.a.L.Int) {
        return b.a.L.Int.wrap(0) + b.a.L.Int.wrap(0);
    }
}

// ----
// f() -> 7
