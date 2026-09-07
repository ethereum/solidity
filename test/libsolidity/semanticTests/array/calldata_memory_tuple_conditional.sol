contract C {
    // Regression test for https://github.com/argotorg/solidity/issues/16066 -
    // assigning a conditional expression whose branches are tuples mixing a memory
    // and a calldata array to a tuple of local variables used to trigger an ICE
    // under --via-ir.
    function f(bool useOps1, uint[] calldata arr) external pure returns (uint sum) {
        uint[] memory ops1 = new uint[](1);
        ops1[0] = 1;
        uint[] memory ops2 = new uint[](1);
        ops2[0] = 2;

        (uint[] memory a, uint[] calldata b) = useOps1 ? (ops1, arr) : (ops2, arr);

        sum = a[0];
        for (uint i = 0; i < b.length; i++)
            sum += b[i];
    }
}
// ----
// f(bool,uint256[]): true, 0x40, 2, 10, 20 -> 31
// f(bool,uint256[]): false, 0x40, 2, 10, 20 -> 32
