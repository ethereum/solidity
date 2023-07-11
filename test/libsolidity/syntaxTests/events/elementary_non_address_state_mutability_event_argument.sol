contract C {
    event a(bool payable);
    event b(string payable);
    event c(int payable);
    event d(int256 payable);
    event e(uint payable);
    event f(uint256 payable);
    event g(bytes1 payable);
    event h(bytes payable);
    event i(bytes32 payable);
    event j(fixed payable);
    event k(fixed80x80 payable);
    event l(ufixed payable);
    event m(ufixed80x80 payable);
}
contract C2 {
    event a(bool view);
    event b(string view);
    event c(int view);
    event d(int256 view);
    event e(uint view);
    event f(uint256 view);
    event g(bytes1 view);
    event h(bytes view);
    event i(bytes32 view);
    event j(fixed view);
    event k(fixed80x80 view);
    event l(ufixed view);
    event m(ufixed80x80 view);
}
contract C3 {
    event a(bool pure);
    event b(string pure);
    event c(int pure);
    event d(int256 pure);
    event e(uint pure);
    event f(uint256 pure);
    event g(bytes1 pure);
    event h(bytes pure);
    event i(bytes32 pure);
    event j(fixed pure);
    event k(fixed80x80 pure);
    event l(ufixed pure);
    event m(ufixed80x80 pure);
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
