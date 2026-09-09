error StringAndUint(string a, uint256 b);
error TwoStrings(string a, string b);
error BytesAndUint(bytes a, uint256 b);
error MixedLocations(string a, uint256 b, bytes c);

contract C {
    string storedString = "from storage";
    bytes storedBytes = hex"aabbccdd";

    function storageString() external view {
        require(false, StringAndUint({b: 42, a: storedString}));
    }

    function memoryString() external pure {
        string memory s = "from memory";
        require(false, StringAndUint({b: 42, a: s}));
    }

    function calldataString(string calldata s) external pure {
        require(false, StringAndUint({b: 42, a: s}));
    }

    function storageAndMemoryStrings() external view {
        string memory m = "from memory";
        require(false, TwoStrings({b: m, a: storedString}));
    }

    function storageBytes() external view {
        require(false, BytesAndUint({b: 99, a: storedBytes}));
    }

    function memoryBytes() external pure {
        bytes memory b = hex"aabbccdd";
        require(false, BytesAndUint({b: 99, a: b}));
    }

    function calldataBytes(bytes calldata b) external pure {
        require(false, BytesAndUint({b: 99, a: b}));
    }

    function mixedLocations() external view {
        bytes memory b = hex"cafe";
        require(false, MixedLocations({c: b, b: 7, a: storedString}));
    }

    function calldataViaMemory(string calldata s) external pure {
        string memory m = s;
        require(false, StringAndUint({b: 42, a: m}));
    }
}

// ----
// storageString() -> FAILURE, hex"81a3bbac", 0x40, 42, 12, "from storage"
// memoryString() -> FAILURE, hex"81a3bbac", 0x40, 42, 11, "from memory"
// calldataString(string): 0x20, 13, "from calldata" -> FAILURE, hex"81a3bbac", 0x40, 42, 13, "from calldata"
// storageAndMemoryStrings() -> FAILURE, hex"16b9a491", 0x40, 0x80, 12, "from storage", 11, "from memory"
// storageBytes() -> FAILURE, hex"2d6fd48a", 0x40, 99, 4, left(0xaabbccdd)
// memoryBytes() -> FAILURE, hex"2d6fd48a", 0x40, 99, 4, left(0xaabbccdd)
// calldataBytes(bytes): 0x20, 4, left(0xaabbccdd) -> FAILURE, hex"2d6fd48a", 0x40, 99, 4, left(0xaabbccdd)
// mixedLocations() -> FAILURE, hex"f8ce5df5", 0x60, 7, 0xa0, 12, "from storage", 2, left(0xcafe)
// calldataViaMemory(string): 0x20, 13, "from calldata" -> FAILURE, hex"81a3bbac", 0x40, 42, 13, "from calldata"
