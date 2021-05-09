pragma abicoder               v2;

contract C {
    struct S { string[] s; }
    function f() public pure returns (S memory) {}
}
// ----
