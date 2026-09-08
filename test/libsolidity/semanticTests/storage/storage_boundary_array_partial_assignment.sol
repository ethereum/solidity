contract C {
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

    function x() public view returns (uint256[10] memory) {
        return getArray()[0];
    }

    function partialAssignArrayBeforeStorageBoundary() public {
        uint256[10][1] storage _x = getArray();
        _x[0] = [21, 22, 23];
    }

    function partialAssignArrayCrossStorageBoundary() public {
        uint256[10][1] storage _x = getArray();
        _x[0] = [11, 12, 13, 14, 15, 16, 17];
    }
}
// ----
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// fillArray()
// gas irOptimized: 220724
// gas legacy: 221456
// gas legacyOptimized: 220893
// gas ssaCFGOptimized: 220723
// x() -> 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
// partialAssignArrayCrossStorageBoundary()
// x() -> 11, 12, 13, 14, 15, 16, 17, 0, 0, 0
// partialAssignArrayBeforeStorageBoundary()
// x() -> 21, 22, 23, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 44180
// gas legacy: 46012
// gas legacyOptimized: 43907
