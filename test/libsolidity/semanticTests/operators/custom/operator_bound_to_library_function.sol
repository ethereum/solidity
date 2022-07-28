==== Source: a.sol ====
library L {
    type Int is int128;

    function add(Int, Int) pure public returns (Int) {
        return Int.wrap(7);
    }

    function sub(Int) pure public returns (Int) {
        return Int.wrap(5);
    }
}
==== Source: b.sol ====
import "a.sol" as a;

contract C {
    using {a.L.add as +} for a.L.Int;
    using {a.L.sub as -} for a.L.Int;

    function f() pure public returns (a.L.Int) {
	return a.L.Int.wrap(0) + a.L.Int.wrap(0);
    }

    function g() pure public returns (a.L.Int) {
	return - a.L.Int.wrap(0);
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 7
// g() -> 5
