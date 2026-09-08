contract C {
    uint256 public y = 42;

    function getArray() internal pure returns (uint256[10][1] storage _x) {
        assembly {
            _x.slot := sub(0, 5)
        }
    }

    function fillArray() public {
        uint256[10][1] storage _x = getArray();
        for (uint i = 1; i < 10; i++)
            _x[0][i] = i;
    }

    function clearArray() public {
        uint256[10][1] storage _x = getArray();
        delete _x[0];
    }

    function x() public view returns (uint256[10] memory) {
        return getArray()[0];
    }
}

// ----
// y() -> 42
// x() -> 0, 0, 0, 0, 0, 42, 0, 0, 0, 0
// fillArray()
// gas irOptimized: 203624
// gas legacy: 204356
// gas legacyOptimized: 203793
// gas ssaCFGOptimized: 203623
// y() -> 5
// x() -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
// clearArray()
// y() -> 0
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 44180
// gas legacy: 46012
// gas legacyOptimized: 43907
