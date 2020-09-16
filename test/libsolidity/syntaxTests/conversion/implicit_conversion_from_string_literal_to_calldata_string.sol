contract C {
    function f1() public pure returns(string calldata) {
        return "hello";
    }

    function f2() public pure returns(string calldata) {
        return unicode"hello";
    }

    function f3() public pure returns(bytes calldata) {
        return hex"68656c6c6f";
    }
}
// ----
// TypeError 6359: (85-92): Return argument type literal_string "hello" is not implicitly convertible to expected type (type of first return variable) string calldata.
// TypeError 6359: (173-187): Return argument type literal_string "hello" is not implicitly convertible to expected type (type of first return variable) string calldata.
// TypeError 6359: (267-282): Return argument type literal_string "hello" is not implicitly convertible to expected type (type of first return variable) bytes calldata.
