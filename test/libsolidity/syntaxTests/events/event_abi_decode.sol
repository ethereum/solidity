contract Test {
    event E(uint);
    function f() public {
        abi.decode(E, (bool));
    }
}
// ----
// TypeError 1956: (80-81): The first argument to "abi.decode" must be implicitly convertible to bytes memory or bytes calldata, but is of type event E(uint256).
