error E(uint);
contract C {
    function f() public pure returns (bytes memory) {
        return abi.encode(E(2));
    }
}
// ----
// TypeError 2056: (108-112='E(2)'): This type cannot be encoded.
