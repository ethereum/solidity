contract C {
    function f() public returns (uint8, uint) {
        // Implicitly convertible, but not exactly the same type.
        try this.f() returns (uint, int x) {

        } catch {

        }
    }
}
// ----
// TypeError: (157-161): Invalid type, expected uint8 but got uint256.
// TypeError: (163-168): Invalid type, expected uint256 but got int256.
