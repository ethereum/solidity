error MyCustomError(uint, bool);
contract Test {
    function f() public {
        abi.decode(MyCustomError, (bool));
    }
}
// ----
// TypeError 1956: (94-107): The first argument to "abi.decode" must be implicitly convertible to bytes memory or bytes calldata, but is of type error MyCustomError(uint256,bool).
