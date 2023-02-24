function suffix(uint) pure suffix returns (string memory) {
    return "suffix";
}

library L {
    function suffix(uint) private pure returns (string memory) {
        return "library function";
    }

    function run() public pure returns (string memory) {
        return suffix(1);
    }
}

contract C {
    function suffix(uint) internal pure returns (string memory) {
        return "contract function";
    }

    function run() public pure returns (string memory) {
        return suffix(1);
    }

    function runL() public pure returns (string memory) {
        return L.run();
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// library: L
// run() -> 0x20, 0x11, "contract function"
// runL() -> 0x20, 0x10, "library function"
