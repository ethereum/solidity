contract C {
    function a() public pure returns (bool payable) {}
    function b() public pure returns (string payable) {}
    function c() public pure returns (int payable) {}
    function d() public pure returns (int256 payable) {}
    function e() public pure returns (uint payable) {}
    function f() public pure returns (uint256 payable) {}
    function g() public pure returns (byte payable) {}
    function h() public pure returns (bytes payable) {}
    function i() public pure returns (bytes32 payable) {}
    function j() public pure returns (fixed payable) {}
    function k() public pure returns (fixed80x80 payable) {}
    function l() public pure returns (ufixed payable) {}
    function m() public pure returns (ufixed80x80 payable) {}
}
// ----
// ParserError: (56-63): State mutability can only be specified for address types.
// ParserError: (113-120): State mutability can only be specified for address types.
// ParserError: (167-174): State mutability can only be specified for address types.
// ParserError: (224-231): State mutability can only be specified for address types.
// ParserError: (279-286): State mutability can only be specified for address types.
// ParserError: (337-344): State mutability can only be specified for address types.
// ParserError: (392-399): State mutability can only be specified for address types.
// ParserError: (448-455): State mutability can only be specified for address types.
// ParserError: (506-513): State mutability can only be specified for address types.
// ParserError: (562-569): State mutability can only be specified for address types.
// ParserError: (623-630): State mutability can only be specified for address types.
// ParserError: (680-687): State mutability can only be specified for address types.
// ParserError: (742-749): State mutability can only be specified for address types.
