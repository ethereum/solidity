==== Source: A ====
type T is uint;
using L for T global;
library L {
    function inc(T x) internal pure returns (T) {
        return T.wrap(T.unwrap(x) + 1);
    }
    function dec(T x) external pure returns (T) {
        return T.wrap(T.unwrap(x) - 1);
    }
}
using {unwrap} for T global;
function unwrap(T x) pure returns (uint) {
    return T.unwrap(x);
}

==== Source: B ====
contract C {
    function f() public pure returns (T r1) {
        r1 = r1.inc().inc();
    }
}

import {T} from "A";

==== Source: C ====
import {C} from "B";

contract D {
    function test() public returns (uint) {
        C c = new C();
        // This tests that bound functions are available
        // even if the type is not available by name.
        // This is a regular function call, a
        // public and an internal library call
        // and a free function call.
        return c.f().inc().inc().dec().unwrap();
    }
}
// ----
// library: "A":L
// test() -> 3
// gas legacy: 130369
