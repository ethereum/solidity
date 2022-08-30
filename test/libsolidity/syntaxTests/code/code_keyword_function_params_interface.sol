interface testInterface {
    function C(function (string code) external) external;
    function D(string code) external;
}

contract testContract {
    function k(string code str) external pure {
        abi.encodeCall(testInterface.D, (str));
    }
}
// ----
// SyntaxError 2397: (51-62): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (99-110): Usage of "code" as a data location is not yet supported.
// SyntaxError 2397: (164-179): Usage of "code" as a data location is not yet supported.
// TypeError 6651: (51-62): Data location must be "memory" or "calldata" for parameter in function, but "code" was given.
// TypeError 6651: (99-110): Data location must be "memory" or "calldata" for parameter in external function, but "code" was given.
// TypeError 6651: (164-179): Data location must be "memory" or "calldata" for parameter in external function, but "code" was given.
