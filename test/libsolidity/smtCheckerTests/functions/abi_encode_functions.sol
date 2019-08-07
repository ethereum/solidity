pragma experimental SMTChecker;pragma experimental ABIEncoderV2;
contract C {
    function f() public pure returns (bytes memory, bytes memory) {
        return (abi.encode(""), abi.encodePacked( "7?8r"));
    }
}
// ----
// Warning: (31-64): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (162-176): Assertion checker does not yet implement this type of function call.
// Warning: (178-203): Assertion checker does not yet implement this type of function call.
