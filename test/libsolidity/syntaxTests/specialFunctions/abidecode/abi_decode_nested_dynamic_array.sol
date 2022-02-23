pragma abicoder v1;
contract C {
    function f() public pure {
        abi.decode("1234", (uint[][3]));
    }
}
// ----
// TypeError 9611: (92-101): Decoding type uint256[] memory[3] memory not supported.
