contract C {
    function f() public {
        uint[] memory x;
        x.pop();
    }
}
// ----
// TypeError 4994: (72-77='x.pop'): Member "pop" is not available in uint256[] memory outside of storage.
