pragma experimental SMTChecker;pragma experimental ABIEncoderV2;
contract C {
    function f() public pure returns (bytes memory, bytes memory) {
        return (abi.encode(""), abi.encodePacked( "7?8r"));
    }
}
// ----
// Warning: (31-64): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (173-175): Assertion checker does not yet support the type of this literal (literal_string "").
// Warning: (162-176): Assertion checker does not yet implement this type of function call.
// Warning: (196-202): Assertion checker does not yet support the type of this literal (literal_string "7?8r").
// Warning: (178-203): Assertion checker does not yet implement this type of function call.
