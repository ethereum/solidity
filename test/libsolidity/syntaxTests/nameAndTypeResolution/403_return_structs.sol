pragma experimental ABIEncoderV2;
contract C {
    struct S { uint a; T[] sub; }
    struct T { uint[] x; }
    function f() public returns (uint, S memory) {
    }
}
// ----
