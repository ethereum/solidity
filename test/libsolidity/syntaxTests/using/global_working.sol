==== Source: A ====
using {f} for S global;
// this should not conflict
using {f} for S;
struct S { uint x; }
function gen() pure returns (S memory) {}
function f(S memory _x) pure returns (uint) { return _x.x; }
==== Source: B ====
function test() pure
{
    uint p = g().f();
    p++;
}
import {gen as g} from "A";
// ----