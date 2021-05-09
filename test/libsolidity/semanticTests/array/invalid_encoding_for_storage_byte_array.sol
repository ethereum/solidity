contract C {
    bytes public x = "abc";
    bytes public y;
    function invalidateXShort() public {
        assembly { sstore(x.slot, 64) }
        delete y;
    }
    function invalidateXLong() public {
        assembly { sstore(x.slot, 5) }
        delete y;
    }
    function abiEncode() public view returns (bytes memory) { return x; }
    function abiEncodePacked() public view returns (bytes memory) { return abi.encodePacked(x); }
    function copyToMemory() public view returns (bytes memory m) { m = x; }
    function indexAccess() public view returns (bytes1) { return x[0]; }
    function assignTo() public { x = "def"; }
    function assignToLong() public { x = "1234567890123456789012345678901234567"; }
    function copyToStorage() public { y = x; }
    function copyFromStorageShort() public { y = "abc"; x = y; }
    function copyFromStorageLong() public { y = "1234567890123456789012345678901234567"; x = y; }
    function arrayPop() public { x.pop(); }
    function arrayPush() public { x.push("t"); }
    function arrayPushEmpty() public { x.push(); }
    function del() public { delete x; }
}
// ====
// compileViaYul: also
// ----
// x() -> 0x20, 3, 0x6162630000000000000000000000000000000000000000000000000000000000
// abiEncode() -> 0x20, 3, 0x6162630000000000000000000000000000000000000000000000000000000000
// abiEncodePacked() -> 0x20, 3, 0x6162630000000000000000000000000000000000000000000000000000000000
// copyToMemory() -> 0x20, 3, 0x6162630000000000000000000000000000000000000000000000000000000000
// indexAccess() -> 0x6100000000000000000000000000000000000000000000000000000000000000
// arrayPushEmpty()
// arrayPush()
// x() -> 0x20, 5, 0x6162630074000000000000000000000000000000000000000000000000000000
// arrayPop()
// assignToLong()
// x() -> 0x20, 0x25, 0x3132333435363738393031323334353637383930313233343536373839303132, 0x3334353637000000000000000000000000000000000000000000000000000000
// assignTo()
// x() -> 0x20, 3, 0x6465660000000000000000000000000000000000000000000000000000000000
// copyFromStorageShort()
// x() -> 0x20, 3, 0x6162630000000000000000000000000000000000000000000000000000000000
// copyFromStorageLong()
// x() -> 0x20, 0x25, 0x3132333435363738393031323334353637383930313233343536373839303132, 0x3334353637000000000000000000000000000000000000000000000000000000
// copyToStorage()
// x() -> 0x20, 0x25, 0x3132333435363738393031323334353637383930313233343536373839303132, 0x3334353637000000000000000000000000000000000000000000000000000000
// y() -> 0x20, 0x25, 0x3132333435363738393031323334353637383930313233343536373839303132, 0x3334353637000000000000000000000000000000000000000000000000000000
// del()
// x() -> 0x20, 0x00
// invalidateXLong()
// x() -> FAILURE, hex"4e487b71", 0x22
// abiEncode() -> FAILURE, hex"4e487b71", 0x22
// abiEncodePacked() -> FAILURE, hex"4e487b71", 0x22
// copyToMemory() -> FAILURE, hex"4e487b71", 0x22
// indexAccess() -> FAILURE, hex"4e487b71", 0x22
// arrayPushEmpty() -> FAILURE, hex"4e487b71", 0x22
// arrayPush() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// arrayPop() -> FAILURE, hex"4e487b71", 0x22
// assignToLong() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// assignTo() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// copyFromStorageShort() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// copyFromStorageLong() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// copyToStorage() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// y() -> 0x20, 0x00
// del() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// invalidateXShort()
// x() -> FAILURE, hex"4e487b71", 0x22
// abiEncode() -> FAILURE, hex"4e487b71", 0x22
// abiEncodePacked() -> FAILURE, hex"4e487b71", 0x22
// copyToMemory() -> FAILURE, hex"4e487b71", 0x22
// indexAccess() -> FAILURE, hex"4e487b71", 0x22
// arrayPushEmpty() -> FAILURE, hex"4e487b71", 0x22
// arrayPush() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// arrayPop() -> FAILURE, hex"4e487b71", 0x22
// assignToLong() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// assignTo() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// copyFromStorageShort() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// copyFromStorageLong() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// copyToStorage() -> FAILURE, hex"4e487b71", 0x22
// x() -> FAILURE, hex"4e487b71", 0x22
// y() -> 0x20, 0x00
