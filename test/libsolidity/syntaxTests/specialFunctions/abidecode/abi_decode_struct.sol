struct S {
    uint x;
}

contract C {
    function f() public pure {
        abi.decode("1234", (S));
    }
}
// ----
// TypeError 9611: (98-99): Decoding type struct S memory not supported.
