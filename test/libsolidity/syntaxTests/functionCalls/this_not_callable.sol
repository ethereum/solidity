contract C {
    function f() public returns (uint, uint) {
        try this() {
        } catch Error(string memory) {
        }
    }
}
// ----
// TypeError 5704: (72-78): This expression is not callable.
