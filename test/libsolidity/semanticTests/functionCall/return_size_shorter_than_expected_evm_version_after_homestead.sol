interface LongReturn {
    function f() external pure returns (uint[20] memory);
}
contract ShortReturn {
    function f() external pure returns (bytes32) {}
}

contract Test {
    function test() public returns (uint) {
        ShortReturn shortReturn = new ShortReturn();
        uint freeMemoryBefore;
        assembly {
            freeMemoryBefore := mload(0x40)
        }

        // This reverts. The external call succeeds but ABI decoding fails due to the returned
        // `bytes32` being much shorter than the expected `uint[20]`.
        LongReturn(address(shortReturn)).f();

        uint freeMemoryAfter;

        assembly {
            freeMemoryAfter := mload(0x40)
        }

        return freeMemoryAfter - freeMemoryBefore;
    }
}
// ====
// EVMVersion: >homestead
// compileViaYul: true
// ----
// test() -> FAILURE
// gas legacy: 131966
