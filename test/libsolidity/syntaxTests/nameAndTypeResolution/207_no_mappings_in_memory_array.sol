contract C {
    function f() public {
        mapping(uint=>uint)[] memory x;
    }
}
// ----
// TypeError: (47-77): Type mapping(uint256 => uint256)[] memory is only valid in storage.
