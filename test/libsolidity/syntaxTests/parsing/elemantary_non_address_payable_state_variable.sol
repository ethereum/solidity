contract C {
    bool payable a;
    string payable b;
    int payable c;
    int256 payable d;
    uint payable e;
    uint256 payable f;
    byte payable g;
    bytes payable h;
    bytes32 payable i;
    fixed payable j;
    fixed80x80 payable k;
    ufixed payable l;
    ufixed80x80 payable m;
}
// ----
// ParserError 9106: (22-29): State mutability can only be specified for address types.
// ParserError 9106: (44-51): State mutability can only be specified for address types.
// ParserError 9106: (63-70): State mutability can only be specified for address types.
// ParserError 9106: (85-92): State mutability can only be specified for address types.
// ParserError 9106: (105-112): State mutability can only be specified for address types.
// ParserError 9106: (128-135): State mutability can only be specified for address types.
// ParserError 9106: (148-155): State mutability can only be specified for address types.
// ParserError 9106: (169-176): State mutability can only be specified for address types.
// ParserError 9106: (192-199): State mutability can only be specified for address types.
// ParserError 9106: (213-220): State mutability can only be specified for address types.
// ParserError 9106: (239-246): State mutability can only be specified for address types.
// ParserError 9106: (261-268): State mutability can only be specified for address types.
// ParserError 9106: (288-295): State mutability can only be specified for address types.
