contract C {
    function f(bytes calldata bytesAsCalldata) external {
        assembly {
            let x := bytesAsCalldata
        }
    }
}
// ----
// TypeError 1397: (111-126): Call data elements cannot be accessed directly. Use ".offset" and ".length" to access the calldata offset and length of this array and then use "calldatacopy".
