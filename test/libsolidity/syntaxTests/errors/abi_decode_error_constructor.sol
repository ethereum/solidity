error E(uint);
contract C {
    function f() public pure returns (bytes memory) {
        return abi.decode(msg.data, (E(1)));
    }
}
// ----
// TypeError 1039: (119-123): Argument has to be a type name.
// TypeError 5132: (90-125): Different number of arguments in return statement than in returns declaration.
