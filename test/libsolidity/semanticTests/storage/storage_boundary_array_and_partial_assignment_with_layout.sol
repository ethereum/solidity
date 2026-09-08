contract C layout at 2**256 - 5 {
    uint256 a;

    function getArray() internal pure returns (uint256[10][1] storage _x) {
        assembly {
            _x.slot := a.slot
        }
    }

    function fillArray() public {
        uint256[10][1] storage _x = getArray();
        for (uint i = 1; i < 10; i++)
            _x[0][i] = i;
    }

    function partialAssignArrayBeforeStorageBoundary() public {
        uint256[10][1] storage _x = getArray();
        _x[0] = [11, 12, 13];
    }

    function partialAssignArrayCrossStorageBoundary() public {
        uint256[10][1] storage _x = getArray();
        _x[0] = [14, 15, 16, 17, 18, 19, 20];
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
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// fillArray()
// gas irOptimized: 220746
// gas legacy: 221473
// gas legacyOptimized: 220915
// gas ssaCFGOptimized: 220745
// partialAssignArrayBeforeStorageBoundary()
// x() -> 11, 12, 13, 0, 0, 0, 0, 0, 0, 0
// fillArray()
// gas irOptimized: 186546
// gas legacy: 187273
// gas legacyOptimized: 186715
// gas ssaCFGOptimized: 186545
// x() -> 11, 1, 2, 3, 4, 5, 6, 7, 8, 9
// partialAssignArrayCrossStorageBoundary()
// x() -> 14, 15, 16, 17, 18, 19, 20, 0, 0, 0
// clearArray()
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 44180
// gas legacy: 46007
// gas legacyOptimized: 43907
