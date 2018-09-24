contract test {

    function oneByteUTF8() public pure returns (bytes32) {
        bytes32 usdollar = "aaa\u0024aaa";
        return usdollar;
    }

    function twoBytesUTF8() public pure returns (bytes32) {
        bytes32 cent = "aaa\u00A2aaa";
        return cent;
    }

    function threeBytesUTF8() public pure returns (bytes32) {
        bytes32 eur = "aaa\u20ACaaa";
        return  eur;
    }

    function together() public pure returns (bytes32) {
        bytes32 res = "\u0024\u00A2\u20AC";
        return res;
    }

    // this function returns an invalid unicode character
    function invalidLiteral() public pure returns(bytes32) {
        bytes32 invalid = "\u00xx";
        return invalid;
    }

}
// ----
// ParserError: (678-681): Expected primary expression.
