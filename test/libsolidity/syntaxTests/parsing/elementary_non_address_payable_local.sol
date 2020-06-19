contract C {
    function f() public pure {
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
}
// ----
// ParserError 9106: (57-64): State mutability can only be specified for address types.
// ParserError 9106: (83-90): State mutability can only be specified for address types.
// ParserError 9106: (106-113): State mutability can only be specified for address types.
// ParserError 9106: (132-139): State mutability can only be specified for address types.
// ParserError 9106: (156-163): State mutability can only be specified for address types.
// ParserError 9106: (183-190): State mutability can only be specified for address types.
// ParserError 9106: (207-214): State mutability can only be specified for address types.
// ParserError 9106: (232-239): State mutability can only be specified for address types.
// ParserError 9106: (259-266): State mutability can only be specified for address types.
// ParserError 9106: (284-291): State mutability can only be specified for address types.
// ParserError 9106: (314-321): State mutability can only be specified for address types.
// ParserError 9106: (340-347): State mutability can only be specified for address types.
// ParserError 9106: (371-378): State mutability can only be specified for address types.
