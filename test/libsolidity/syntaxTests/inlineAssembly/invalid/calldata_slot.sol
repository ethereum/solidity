contract C {
    function f(bytes calldata bytesAsCalldata) external {
        assembly {
            let x := bytesAsCalldata.slot
        }
    }
}
// ----
// TypeError 1536: (111-131): Calldata variables only support ".offset" and ".length".
