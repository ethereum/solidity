contract C {
    function f() public {
        uint[] memory x;
        x.pop();
    }
}
// ----
// TypeError: (72-77): Member "pop" is not available in uint256[] memory outside of storage.
