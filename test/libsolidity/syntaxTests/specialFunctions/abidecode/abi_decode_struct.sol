pragma abicoder v1;
struct S {
    uint x;
}

contract C {
    function f() public pure {
        abi.decode("1234", (S));
    }
}
// ----
// TypeError 9611: (118-119): Decoding type struct S memory not supported.
