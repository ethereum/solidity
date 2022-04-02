contract C {
    function f() public {
        uint[] memory x;
        x.push();
    }
}
// ----
// TypeError 4994: (72-78='x.push'): Member "push" is not available in uint256[] memory outside of storage.
