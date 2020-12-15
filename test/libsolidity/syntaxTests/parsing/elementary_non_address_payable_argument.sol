contract C {
    function a(bool payable) public pure {}
    function b(string payable) public pure {}
    function c(int payable) public pure {}
    function d(int256 payable) public pure {}
    function e(uint payable) public pure {}
    function f(uint256 payable) public pure {}
    function g(bytes1 payable) public pure {}
    function h(bytes payable) public pure {}
    function i(bytes32 payable) public pure {}
    function j(fixed payable) public pure {}
    function k(fixed80x80 payable) public pure {}
    function l(ufixed payable) public pure {}
    function m(ufixed80x80 payable) public pure {}
}
// ----
// ParserError 9106: (33-40): State mutability can only be specified for address types.
// ParserError 9106: (79-86): State mutability can only be specified for address types.
// ParserError 9106: (122-129): State mutability can only be specified for address types.
// ParserError 9106: (168-175): State mutability can only be specified for address types.
// ParserError 9106: (212-219): State mutability can only be specified for address types.
// ParserError 9106: (259-266): State mutability can only be specified for address types.
// ParserError 9106: (305-312): State mutability can only be specified for address types.
// ParserError 9106: (350-357): State mutability can only be specified for address types.
// ParserError 9106: (397-404): State mutability can only be specified for address types.
// ParserError 9106: (442-449): State mutability can only be specified for address types.
// ParserError 9106: (492-499): State mutability can only be specified for address types.
// ParserError 9106: (538-545): State mutability can only be specified for address types.
// ParserError 9106: (589-596): State mutability can only be specified for address types.
