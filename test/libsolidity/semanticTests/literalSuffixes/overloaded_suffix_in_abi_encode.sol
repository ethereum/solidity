function suffix(uint x) pure suffix returns (uint) { return x; }
function suffix(bool x) pure suffix returns (bool) { return x; }
function suffix(address x) pure suffix returns (address) { return x; }
function suffix(string memory x) pure suffix returns (string memory) { return x; }

contract C {
    function run() public returns (bytes memory) {
        return abi.encode(
            42 suffix,
            true suffix,
            0x1234567890123456789012345678901234567890 suffix,
            "a" suffix
        );
    }
}
// ----
// run() -> 0x20, 0xc0, 0x2a, 1, 0x1234567890123456789012345678901234567890, 0x80, 1, "a"
