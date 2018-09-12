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
// ParserError: (57-64): State mutability can only be specified for address types.
// ParserError: (83-90): State mutability can only be specified for address types.
// ParserError: (106-113): State mutability can only be specified for address types.
// ParserError: (132-139): State mutability can only be specified for address types.
// ParserError: (156-163): State mutability can only be specified for address types.
// ParserError: (183-190): State mutability can only be specified for address types.
// ParserError: (207-214): State mutability can only be specified for address types.
// ParserError: (232-239): State mutability can only be specified for address types.
// ParserError: (259-266): State mutability can only be specified for address types.
// ParserError: (284-291): State mutability can only be specified for address types.
// ParserError: (314-321): State mutability can only be specified for address types.
// ParserError: (340-347): State mutability can only be specified for address types.
// ParserError: (371-378): State mutability can only be specified for address types.
