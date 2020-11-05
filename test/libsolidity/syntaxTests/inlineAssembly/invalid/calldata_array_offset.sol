contract C {
    function f(uint[] calldata bytesAsCalldata) external pure {
        assembly {
            let x := bytesAsCalldata.offset
        }
    }
}
// ----
