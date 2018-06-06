pragma experimental "v0.5.0";
contract C {
    function f(bytes bytesAsCalldata) external {
        assembly {
            let x := bytesAsCalldata
        }
    }
}
// ----
// TypeError: (132-147): Call data elements cannot be accessed directly. Copy to a local variable first or use "calldataload" or "calldatacopy" with manually determined offsets and sizes.
