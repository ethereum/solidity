contract C {
    function f() public {
        mapping(uint=>uint)[] memory x;
    }
}
// ----
// TypeError 4061: (47-77): Type mapping(uint256 => uint256)[] is only valid in storage because it contains a (nested) mapping.
