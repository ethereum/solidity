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

        LongReturn(address(shortReturn)).f();

        uint freeMemoryAfter;

        assembly {
            freeMemoryAfter := mload(0x40)
        }

        return freeMemoryAfter - freeMemoryBefore;
    }
}
// ====
// EVMVersion: <=homestead
// compileViaYul: true
// ----
// test() -> 0x0500
// gas legacy: 131966
