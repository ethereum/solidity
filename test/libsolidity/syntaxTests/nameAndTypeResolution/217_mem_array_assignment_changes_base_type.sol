contract C {
    function f(uint8[] memory x) private {
        // Such an assignment is possible in storage, but not in memory
        // (because it would incur an otherwise unnecessary copy).
        // This requirement might be lifted, though.
        uint[] memory y = x;
    }
}
// ----
// TypeError: (256-275): Type uint8[] memory is not implicitly convertible to expected type uint256[] memory.
