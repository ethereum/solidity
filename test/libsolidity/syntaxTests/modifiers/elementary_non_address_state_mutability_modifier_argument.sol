contract C {
    modifier a(bool payable) {}
    modifier b(string payable) {}
    modifier c(int payable) {}
    modifier d(int256 payable) {}
    modifier e(uint payable) {}
    modifier f(uint256 payable) {}
    modifier g(bytes1 payable) {}
    modifier h(bytes payable) {}
    modifier i(bytes32 payable) {}
    modifier j(fixed payable) {}
    modifier k(fixed80x80 payable) {}
    modifier l(ufixed payable) {}
    modifier m(ufixed80x80 payable) {}
}
contract C2 {
    modifier a(bool view) {}
    modifier b(string view) {}
    modifier c(int view) {}
    modifier d(int256 view) {}
    modifier e(uint view) {}
    modifier f(uint256 view) {}
    modifier g(bytes1 view) {}
    modifier h(bytes view) {}
    modifier i(bytes32 view) {}
    modifier j(fixed view) {}
    modifier k(fixed80x80 view) {}
    modifier l(ufixed view) {}
    modifier m(ufixed80x80 view) {}
}
contract C3 {
    modifier a(bool pure) {}
    modifier b(string pure) {}
    modifier c(int pure) {}
    modifier d(int256 pure) {}
    modifier e(uint pure) {}
    modifier f(uint256 pure) {}
    modifier g(bytes1 pure) {}
    modifier h(bytes pure) {}
    modifier i(bytes32 pure) {}
    modifier j(fixed pure) {}
    modifier k(fixed80x80 pure) {}
    modifier l(ufixed pure) {}
    modifier m(ufixed80x80 pure) {}
}
// ----
// ParserError 9106: (33-40): State mutability can only be specified for address types.
// ParserError 9106: (67-74): State mutability can only be specified for address types.
// ParserError 9106: (98-105): State mutability can only be specified for address types.
// ParserError 9106: (132-139): State mutability can only be specified for address types.
// ParserError 9106: (164-171): State mutability can only be specified for address types.
// ParserError 9106: (199-206): State mutability can only be specified for address types.
// ParserError 9106: (233-240): State mutability can only be specified for address types.
// ParserError 9106: (266-273): State mutability can only be specified for address types.
// ParserError 9106: (301-308): State mutability can only be specified for address types.
// ParserError 9106: (334-341): State mutability can only be specified for address types.
// ParserError 9106: (372-379): State mutability can only be specified for address types.
// ParserError 9106: (406-413): State mutability can only be specified for address types.
// ParserError 9106: (445-452): State mutability can only be specified for address types.
// ParserError 9106: (493-497): State mutability can only be specified for address types.
// ParserError 9106: (524-528): State mutability can only be specified for address types.
// ParserError 9106: (552-556): State mutability can only be specified for address types.
// ParserError 9106: (583-587): State mutability can only be specified for address types.
// ParserError 9106: (612-616): State mutability can only be specified for address types.
// ParserError 9106: (644-648): State mutability can only be specified for address types.
// ParserError 9106: (675-679): State mutability can only be specified for address types.
// ParserError 9106: (705-709): State mutability can only be specified for address types.
// ParserError 9106: (737-741): State mutability can only be specified for address types.
// ParserError 9106: (767-771): State mutability can only be specified for address types.
// ParserError 9106: (802-806): State mutability can only be specified for address types.
// ParserError 9106: (833-837): State mutability can only be specified for address types.
// ParserError 9106: (869-873): State mutability can only be specified for address types.
// ParserError 9106: (914-918): State mutability can only be specified for address types.
// ParserError 9106: (945-949): State mutability can only be specified for address types.
// ParserError 9106: (973-977): State mutability can only be specified for address types.
// ParserError 9106: (1004-1008): State mutability can only be specified for address types.
// ParserError 9106: (1033-1037): State mutability can only be specified for address types.
// ParserError 9106: (1065-1069): State mutability can only be specified for address types.
// ParserError 9106: (1096-1100): State mutability can only be specified for address types.
// ParserError 9106: (1126-1130): State mutability can only be specified for address types.
// ParserError 9106: (1158-1162): State mutability can only be specified for address types.
// ParserError 9106: (1188-1192): State mutability can only be specified for address types.
// ParserError 9106: (1223-1227): State mutability can only be specified for address types.
// ParserError 9106: (1254-1258): State mutability can only be specified for address types.
// ParserError 9106: (1290-1294): State mutability can only be specified for address types.
