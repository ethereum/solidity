contract C {
    function f() public {
        uint[] memory x;
        x.push();
    }
}
// ----
// TypeError: (72-78): Member "push" is not available in uint256[] memory outside of storage.
