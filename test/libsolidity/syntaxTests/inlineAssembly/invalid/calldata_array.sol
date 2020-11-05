contract C {
    function f(uint[] calldata bytesAsCalldata) external {
        assembly {
            let x := bytesAsCalldata
        }
    }
}
// ----
// TypeError 1397: (112-127): Call data elements cannot be accessed directly. Use ".offset" and ".length" to access the calldata offset and length of this array and then use "calldatacopy".
