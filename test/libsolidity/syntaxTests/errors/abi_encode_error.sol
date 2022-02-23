error E(uint);
contract C {
    function f() public pure returns (bytes memory) {
        return abi.encode(E);
    }
}
// ----
// TypeError 2056: (108-109): This type cannot be encoded.
