error E(uint);
contract C {
    function f() public pure returns (bytes memory) {
        return abi.decode(msg.data, (E));
    }
}
// ----
// TypeError 1039: (119-120): Argument has to be a type name.
// TypeError 5132: (90-122): Different number of arguments in return statement than in returns declaration.
