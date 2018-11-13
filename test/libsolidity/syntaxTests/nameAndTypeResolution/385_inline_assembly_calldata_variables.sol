contract C {
    function f(bytes calldata bytesAsCalldata) external {
        assembly {
            let x := bytesAsCalldata
        }
    }
}
// ----
// TypeError: (111-126): Call data elements cannot be accessed directly. Copy to a local variable first or use "calldataload" or "calldatacopy" with manually determined offsets and sizes.
