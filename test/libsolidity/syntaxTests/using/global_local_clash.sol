==== Source: A ====
using {f} for S global;
struct S { uint x; }
function gen() pure returns (S memory) {}
function f(S memory _x) pure returns (uint) { return _x.x; }
function f1(S memory _x) pure returns (uint) { return _x.x + 1; }
==== Source: B ====
contract C {
    // Here, f points to f1, so we end up with two different functions
    // bound as S.f
    using {f} for S;

    function test() pure public
    {
        uint p = g().f();
    }
}
import {gen as g, f1 as f, S} from "A";
import "A" as A;
// ----
// TypeError 6675: (B:181-186): Member "f" not unique after argument-dependent lookup in struct S memory.
