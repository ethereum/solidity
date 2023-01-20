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
contract C1 {
    function a(bool view) public pure {}
    function b(string view) public pure {}
    function c(int view) public pure {}
    function d(int256 view) public pure {}
    function e(uint view) public pure {}
    function f(uint256 view) public pure {}
    function g(bytes1 view) public pure {}
    function h(bytes view) public pure {}
    function i(bytes32 view) public pure {}
    function j(fixed view) public pure {}
    function k(fixed80x80 view) public pure {}
    function l(ufixed view) public pure {}
    function m(ufixed80x80 view) public pure {}
}
contract C2 {
    function a(bool pure) public pure {}
    function b(string pure) public pure {}
    function c(int pure) public pure {}
    function d(int256 pure) public pure {}
    function e(uint pure) public pure {}
    function f(uint256 pure) public pure {}
    function g(bytes1 pure) public pure {}
    function h(bytes pure) public pure {}
    function i(bytes32 pure) public pure {}
    function j(fixed pure) public pure {}
    function k(fixed80x80 pure) public pure {}
    function l(ufixed pure) public pure {}
    function m(ufixed80x80 pure) public pure {}
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
// ParserError 9106: (649-653): State mutability can only be specified for address types.
// ParserError 9106: (692-696): State mutability can only be specified for address types.
// ParserError 9106: (732-736): State mutability can only be specified for address types.
// ParserError 9106: (775-779): State mutability can only be specified for address types.
// ParserError 9106: (816-820): State mutability can only be specified for address types.
// ParserError 9106: (860-864): State mutability can only be specified for address types.
// ParserError 9106: (903-907): State mutability can only be specified for address types.
// ParserError 9106: (945-949): State mutability can only be specified for address types.
// ParserError 9106: (989-993): State mutability can only be specified for address types.
// ParserError 9106: (1031-1035): State mutability can only be specified for address types.
// ParserError 9106: (1078-1082): State mutability can only be specified for address types.
// ParserError 9106: (1121-1125): State mutability can only be specified for address types.
// ParserError 9106: (1169-1173): State mutability can only be specified for address types.
// ParserError 9106: (1226-1230): State mutability can only be specified for address types.
// ParserError 9106: (1269-1273): State mutability can only be specified for address types.
// ParserError 9106: (1309-1313): State mutability can only be specified for address types.
// ParserError 9106: (1352-1356): State mutability can only be specified for address types.
// ParserError 9106: (1393-1397): State mutability can only be specified for address types.
// ParserError 9106: (1437-1441): State mutability can only be specified for address types.
// ParserError 9106: (1480-1484): State mutability can only be specified for address types.
// ParserError 9106: (1522-1526): State mutability can only be specified for address types.
// ParserError 9106: (1566-1570): State mutability can only be specified for address types.
// ParserError 9106: (1608-1612): State mutability can only be specified for address types.
// ParserError 9106: (1655-1659): State mutability can only be specified for address types.
// ParserError 9106: (1698-1702): State mutability can only be specified for address types.
// ParserError 9106: (1746-1750): State mutability can only be specified for address types.
