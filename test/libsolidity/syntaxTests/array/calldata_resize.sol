contract C {
    function f (uint256[] calldata x) external pure {
        x.length = 42;
    }
}
// ----
// TypeError: (75-83): Calldata arrays cannot be resized.
