contract C {
    uint8[3] st = [1, 2, 3];
    uint8[] public dt = [4, 5, 6];

    function s() public returns (uint8[3] memory) {
        return st;
    }

    function d() public returns (uint8[] memory) {
	return dt;
    }
}
// ====
// EVMVersion: >=constantinople
// compileToEwasm: also
// compileViaYul: also
// ----
// s() -> 1, 2, 3
// d() -> 0x20, 3, 4, 5, 6

