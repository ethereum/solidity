pragma abicoder               v2;

contract C {
    function f() public pure {
        abi.encode([new uint[](5), new uint[](7)]);
    }
}
// ----
// Warning 6133: (87-129): Statement has no effect.
