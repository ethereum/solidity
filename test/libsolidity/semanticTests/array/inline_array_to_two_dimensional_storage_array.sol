pragma abicoder v2;

contract C {
    uint8[3][2] st = [[1, 2, 3], [4, 5, 6]];
    uint8[][] dt = [[1, 2], [3, 4, 5, 6]];

    function s() public returns (uint8[3][2] memory) {
        return st;
    }

    function d() public returns (uint8[][] memory) {
        return dt;
    }
}

// ====
// compileViaYul: true
// ----
// s() -> 1, 2, 3, 4, 5, 6
// d() -> 0x20, 2, 0x40, 0xa0, 2, 1, 2, 4, 3, 4, 5, 6
