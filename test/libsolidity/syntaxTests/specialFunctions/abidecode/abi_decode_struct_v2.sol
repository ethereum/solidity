pragma abicoder               v2;

struct S {
    uint x;
}

contract C {
    function f() public pure {
        abi.decode("1234", (S));
    }
}
// ----
// Warning 6133: (113-136='abi.decode("1234", (S))'): Statement has no effect.
