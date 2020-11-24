pragma abicoder               v2;
library c {
    struct S { uint x; }
    function f() public returns (S memory) {}
}
// ----
