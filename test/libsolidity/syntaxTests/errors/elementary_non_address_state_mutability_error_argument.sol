contract C {
    error a(bool payable);
    error b(string payable);
    error c(int payable);
    error d(int256 payable);
    error e(uint payable);
    error f(uint256 payable);
    error g(bytes1 payable);
    error h(bytes payable);
    error i(bytes32 payable);
    error j(fixed payable);
    error k(fixed80x80 payable);
    error l(ufixed payable);
    error m(ufixed80x80 payable);
}
contract C2 {
    error a(bool view);
    error b(string view);
    error c(int view);
    error d(int256 view);
    error e(uint view);
    error f(uint256 view);
    error g(bytes1 view);
    error h(bytes view);
    error i(bytes32 view);
    error j(fixed view);
    error k(fixed80x80 view);
    error l(ufixed view);
    error m(ufixed80x80 view);
}
contract C3 {
    error a(bool pure);
    error b(string pure);
    error c(int pure);
    error d(int256 pure);
    error e(uint pure);
    error f(uint256 pure);
    error g(bytes1 pure);
    error h(bytes pure);
    error i(bytes32 pure);
    error j(fixed pure);
    error k(fixed80x80 pure);
    error l(ufixed pure);
    error m(ufixed80x80 pure);
}
// ----
// ParserError 9106: (30-37): State mutability can only be specified for address types.
// ParserError 9106: (59-66): State mutability can only be specified for address types.
// ParserError 9106: (85-92): State mutability can only be specified for address types.
// ParserError 9106: (114-121): State mutability can only be specified for address types.
// ParserError 9106: (141-148): State mutability can only be specified for address types.
// ParserError 9106: (171-178): State mutability can only be specified for address types.
// ParserError 9106: (200-207): State mutability can only be specified for address types.
// ParserError 9106: (228-235): State mutability can only be specified for address types.
// ParserError 9106: (258-265): State mutability can only be specified for address types.
// ParserError 9106: (286-293): State mutability can only be specified for address types.
// ParserError 9106: (319-326): State mutability can only be specified for address types.
// ParserError 9106: (348-355): State mutability can only be specified for address types.
// ParserError 9106: (382-389): State mutability can only be specified for address types.
// ParserError 9106: (425-429): State mutability can only be specified for address types.
// ParserError 9106: (451-455): State mutability can only be specified for address types.
// ParserError 9106: (474-478): State mutability can only be specified for address types.
// ParserError 9106: (500-504): State mutability can only be specified for address types.
// ParserError 9106: (524-528): State mutability can only be specified for address types.
// ParserError 9106: (551-555): State mutability can only be specified for address types.
// ParserError 9106: (577-581): State mutability can only be specified for address types.
// ParserError 9106: (602-606): State mutability can only be specified for address types.
// ParserError 9106: (629-633): State mutability can only be specified for address types.
// ParserError 9106: (654-658): State mutability can only be specified for address types.
// ParserError 9106: (684-688): State mutability can only be specified for address types.
// ParserError 9106: (710-714): State mutability can only be specified for address types.
// ParserError 9106: (741-745): State mutability can only be specified for address types.
// ParserError 9106: (781-785): State mutability can only be specified for address types.
// ParserError 9106: (807-811): State mutability can only be specified for address types.
// ParserError 9106: (830-834): State mutability can only be specified for address types.
// ParserError 9106: (856-860): State mutability can only be specified for address types.
// ParserError 9106: (880-884): State mutability can only be specified for address types.
// ParserError 9106: (907-911): State mutability can only be specified for address types.
// ParserError 9106: (933-937): State mutability can only be specified for address types.
// ParserError 9106: (958-962): State mutability can only be specified for address types.
// ParserError 9106: (985-989): State mutability can only be specified for address types.
// ParserError 9106: (1010-1014): State mutability can only be specified for address types.
// ParserError 9106: (1040-1044): State mutability can only be specified for address types.
// ParserError 9106: (1066-1070): State mutability can only be specified for address types.
// ParserError 9106: (1097-1101): State mutability can only be specified for address types.
