pragma abicoder               v2;

contract C {
    function f() public pure {
        abi.decode("1234", (uint[][3]));
    }
}
// ----
// Warning 6133: (87-118): Statement has no effect.
