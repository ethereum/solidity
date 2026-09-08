contract C {
    function getArray() internal pure returns (uint256[10][1] storage _x) {
        assembly {
            _x.slot := sub(0, 5)
        }
    }

    function assignArray(uint256[10] memory y) public {
        uint256[10][1] storage _x = getArray();
        _x[0] = y;
    }

    function x() public view returns (uint256[10] memory) {
        return getArray()[0];
    }
}
// ----
// x() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// assignArray(uint256[10]): 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ->
// gas irOptimized: 245233
// gas legacy: 249351
// gas legacyOptimized: 245365
// gas ssaCFGOptimized: 245232
// x() -> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
// assignArray(uint256[10]): 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 ->
// x() -> 10, 20, 30, 40, 50, 60, 70, 80, 90, 100
// gas irOptimized: 44180
// gas legacy: 46012
// gas legacyOptimized: 43907
