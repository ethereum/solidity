function f(S storage g) view returns (uint) { S storage t = g; return t.x; }
struct S { uint x; }
// ----
