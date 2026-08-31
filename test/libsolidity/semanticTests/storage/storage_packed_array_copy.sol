contract C {
    bytes8[9] _x; // 4 per slot
    bytes17[10] _y; // 1 per slot, no offset counter

	constructor() {
        for (uint256 i = 0; i < _x.length; ++i) _x[i] = bytes8(uint64(i));
        _y[8] = _y[9] = bytes8(uint64(2));
	}

    function getXAsUint() public view returns (uint64[9] memory result) {
        for (uint i = 0; i < 9; i++) {
            result[i] = uint64(_x[i]);
        }
    }

    function getYAsUint() public view returns (uint64[10] memory result) {
        for (uint i = 0; i < 10; i++) {
            result[i] = uint64(bytes8(_y[i]));
        }
    }

    function copy() public {
        _y = _x;
    }
}

// ----
// getXAsUint() -> 0, 1, 2, 3, 4, 5, 6, 7, 8
// getYAsUint() -> 0, 0, 0, 0, 0, 0, 0, 0, 2, 2
// copy()
// gas irOptimized: 190810
// gas legacy: 195580
// gas legacyOptimized: 190906
// gas ssaCFGOptimized: 191096
// getXAsUint() -> 0, 1, 2, 3, 4, 5, 6, 7, 8
// getYAsUint() -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 0
