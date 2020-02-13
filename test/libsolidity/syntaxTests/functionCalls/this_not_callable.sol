contract C {
    function f() public returns (uint, uint) {
        try this() {
        } catch Error(string memory) {
        }
    }
}
// ----
// TypeError: (72-78): Type is not callable
