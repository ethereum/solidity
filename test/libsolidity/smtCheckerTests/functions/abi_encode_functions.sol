pragma experimental SMTChecker;pragma experimental ABIEncoderV2;
contract C {
    function f() public pure returns (bytes memory, bytes memory) {
        return (abi.encode(""), abi.encodePacked( "7?8r"));
    }
}
// ----
// Warning 8364: (162-165): Assertion checker does not yet implement type abi
// Warning 4588: (162-176): Assertion checker does not yet implement this type of function call.
// Warning 8364: (178-181): Assertion checker does not yet implement type abi
// Warning 4588: (178-203): Assertion checker does not yet implement this type of function call.
