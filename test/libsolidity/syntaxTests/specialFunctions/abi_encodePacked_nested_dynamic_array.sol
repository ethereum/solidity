pragma abicoder v1;
contract C {
    function f() public pure {
        abi.encodePacked([new uint[](5), new uint[](7)]);
    }
}
// ----
// TypeError 9578: (89-119): Type not supported in packed mode.
