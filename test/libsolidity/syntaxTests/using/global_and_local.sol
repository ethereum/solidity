==== Source: A ====
using {f} for S global;
struct S { uint x; }
function gen() pure returns (S memory) {}
function f(S memory _x) pure returns (uint) { return _x.x; }
==== Source: B ====
contract C {
    using {fun} for S;
    // Adds the same function again with the same name,
    // so it's fine.
    using {A.f} for S;

    function test() pure public
    {
        uint p = g().f();
        p = g().fun();
    }
}
import {gen as g, f as fun, S} from "A";
import "A" as A;
// ----