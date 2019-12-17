contract C {
    function f(bytes calldata x) external {
        return this.f(x[1:2]);
    }
}
// ----
// TypeError: (79-85): Invalid type for argument in function call. Invalid implicit conversion from bytes calldata slice to bytes memory requested.
