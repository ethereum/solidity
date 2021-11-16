==== Source: A ====
using {f} for S global;
using {g} for S;
struct S { uint x; }
function gen() pure returns (S memory) {}
function f(S memory _x) pure { _x.g(); }
function g(S memory _x) pure { }
==== Source: B ====
import "A";
function test() pure
{
    gen().f();
    gen().g();
}

// ----
// TypeError 9582: (B:54-61): Member "g" not found or not visible after argument-dependent lookup in struct S memory.
