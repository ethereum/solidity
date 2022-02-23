contract C {
    function f() public pure {
        bool payable a;
        string payable b;
        int payable c;
        int256 payable d;
        uint payable e;
        uint256 payable f;
        bytes1 payable g;
        bytes payable h;
        bytes32 payable i;
        fixed payable j;
        fixed80x80 payable k;
        ufixed payable l;
        ufixed80x80 payable m;
    }
}
// ----
// ParserError 9106: (57-64): State mutability can only be specified for address types.
// ParserError 9106: (83-90): State mutability can only be specified for address types.
// ParserError 9106: (106-113): State mutability can only be specified for address types.
// ParserError 9106: (132-139): State mutability can only be specified for address types.
// ParserError 9106: (156-163): State mutability can only be specified for address types.
// ParserError 9106: (183-190): State mutability can only be specified for address types.
// ParserError 9106: (209-216): State mutability can only be specified for address types.
// ParserError 9106: (234-241): State mutability can only be specified for address types.
// ParserError 9106: (261-268): State mutability can only be specified for address types.
// ParserError 9106: (286-293): State mutability can only be specified for address types.
// ParserError 9106: (316-323): State mutability can only be specified for address types.
// ParserError 9106: (342-349): State mutability can only be specified for address types.
// ParserError 9106: (373-380): State mutability can only be specified for address types.
