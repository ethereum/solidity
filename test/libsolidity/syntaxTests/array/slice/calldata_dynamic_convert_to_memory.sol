contract C {
    function f(bytes calldata x) external {
        bytes memory y = x[1:2];
    }
}
// ----
// TypeError: (65-88): Type bytes calldata slice is not implicitly convertible to expected type bytes memory.
