contract C {
    function a() public pure returns (bool payable) {}
    function b() public pure returns (string payable) {}
    function c() public pure returns (int payable) {}
    function d() public pure returns (int256 payable) {}
    function e() public pure returns (uint payable) {}
    function f() public pure returns (uint256 payable) {}
    function g() public pure returns (bytes1 payable) {}
    function h() public pure returns (bytes payable) {}
    function i() public pure returns (bytes32 payable) {}
    function j() public pure returns (fixed payable) {}
    function k() public pure returns (fixed80x80 payable) {}
    function l() public pure returns (ufixed payable) {}
    function m() public pure returns (ufixed80x80 payable) {}
}
contract C1 {
    function a() public pure returns (bool view) {}
    function b() public pure returns (string view) {}
    function c() public pure returns (int view) {}
    function d() public pure returns (int256 view) {}
    function e() public pure returns (uint view) {}
    function f() public pure returns (uint256 view) {}
    function g() public pure returns (bytes1 view) {}
    function h() public pure returns (bytes view) {}
    function i() public pure returns (bytes32 view) {}
    function j() public pure returns (fixed view) {}
    function k() public pure returns (fixed80x80 view) {}
    function l() public pure returns (ufixed view) {}
    function m() public pure returns (ufixed80x80 view) {}
}
contract C2 {
    function a() public pure returns (bool pure) {}
    function b() public pure returns (string pure) {}
    function c() public pure returns (int pure) {}
    function d() public pure returns (int256 pure) {}
    function e() public pure returns (uint pure) {}
    function f() public pure returns (uint256 pure) {}
    function g() public pure returns (bytes1 pure) {}
    function h() public pure returns (bytes pure) {}
    function i() public pure returns (bytes32 pure) {}
    function j() public pure returns (fixed pure) {}
    function k() public pure returns (fixed80x80 pure) {}
    function l() public pure returns (ufixed pure) {}
    function m() public pure returns (ufixed80x80 pure) {}
}
// ----
// ParserError 9106: (56-63): State mutability can only be specified for address types.
// ParserError 9106: (113-120): State mutability can only be specified for address types.
// ParserError 9106: (167-174): State mutability can only be specified for address types.
// ParserError 9106: (224-231): State mutability can only be specified for address types.
// ParserError 9106: (279-286): State mutability can only be specified for address types.
// ParserError 9106: (337-344): State mutability can only be specified for address types.
// ParserError 9106: (394-401): State mutability can only be specified for address types.
// ParserError 9106: (450-457): State mutability can only be specified for address types.
// ParserError 9106: (508-515): State mutability can only be specified for address types.
// ParserError 9106: (564-571): State mutability can only be specified for address types.
// ParserError 9106: (625-632): State mutability can only be specified for address types.
// ParserError 9106: (682-689): State mutability can only be specified for address types.
// ParserError 9106: (744-751): State mutability can only be specified for address types.
// ParserError 9106: (815-819): State mutability can only be specified for address types.
// ParserError 9106: (869-873): State mutability can only be specified for address types.
// ParserError 9106: (920-924): State mutability can only be specified for address types.
// ParserError 9106: (974-978): State mutability can only be specified for address types.
// ParserError 9106: (1026-1030): State mutability can only be specified for address types.
// ParserError 9106: (1081-1085): State mutability can only be specified for address types.
// ParserError 9106: (1135-1139): State mutability can only be specified for address types.
// ParserError 9106: (1188-1192): State mutability can only be specified for address types.
// ParserError 9106: (1243-1247): State mutability can only be specified for address types.
// ParserError 9106: (1296-1300): State mutability can only be specified for address types.
// ParserError 9106: (1354-1358): State mutability can only be specified for address types.
// ParserError 9106: (1408-1412): State mutability can only be specified for address types.
// ParserError 9106: (1467-1471): State mutability can only be specified for address types.
// ParserError 9106: (1535-1539): State mutability can only be specified for address types.
// ParserError 9106: (1589-1593): State mutability can only be specified for address types.
// ParserError 9106: (1640-1644): State mutability can only be specified for address types.
// ParserError 9106: (1694-1698): State mutability can only be specified for address types.
// ParserError 9106: (1746-1750): State mutability can only be specified for address types.
// ParserError 9106: (1801-1805): State mutability can only be specified for address types.
// ParserError 9106: (1855-1859): State mutability can only be specified for address types.
// ParserError 9106: (1908-1912): State mutability can only be specified for address types.
// ParserError 9106: (1963-1967): State mutability can only be specified for address types.
// ParserError 9106: (2016-2020): State mutability can only be specified for address types.
// ParserError 9106: (2074-2078): State mutability can only be specified for address types.
// ParserError 9106: (2128-2132): State mutability can only be specified for address types.
// ParserError 9106: (2187-2191): State mutability can only be specified for address types.
