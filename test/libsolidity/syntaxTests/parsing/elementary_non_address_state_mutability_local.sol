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
contract C2 {
    function f() public pure {
        bool view a;
        string view b;
        int view c;
        int256 view d;
        uint view e;
        uint256 view f;
        bytes1 view g;
        bytes view h;
        bytes32 view i;
        fixed view j;
        fixed80x80 view k;
        ufixed view l;
        ufixed80x80 view m;
    }
}
contract C3 {
    function f() public pure {
        bool pure a;
        string pure b;
        int pure c;
        int256 pure d;
        uint pure e;
        uint256 pure f;
        bytes1 pure g;
        bytes pure h;
        bytes32 pure i;
        fixed pure j;
        fixed80x80 pure k;
        ufixed pure l;
        ufixed80x80 pure m;
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
// ParserError 9106: (450-454): State mutability can only be specified for address types.
// ParserError 9106: (473-477): State mutability can only be specified for address types.
// ParserError 9106: (493-497): State mutability can only be specified for address types.
// ParserError 9106: (516-520): State mutability can only be specified for address types.
// ParserError 9106: (537-541): State mutability can only be specified for address types.
// ParserError 9106: (561-565): State mutability can only be specified for address types.
// ParserError 9106: (584-588): State mutability can only be specified for address types.
// ParserError 9106: (606-610): State mutability can only be specified for address types.
// ParserError 9106: (630-634): State mutability can only be specified for address types.
// ParserError 9106: (652-656): State mutability can only be specified for address types.
// ParserError 9106: (679-683): State mutability can only be specified for address types.
// ParserError 9106: (702-706): State mutability can only be specified for address types.
// ParserError 9106: (730-734): State mutability can only be specified for address types.
// ParserError 9106: (804-808): State mutability can only be specified for address types.
// ParserError 9106: (827-831): State mutability can only be specified for address types.
// ParserError 9106: (847-851): State mutability can only be specified for address types.
// ParserError 9106: (870-874): State mutability can only be specified for address types.
// ParserError 9106: (891-895): State mutability can only be specified for address types.
// ParserError 9106: (915-919): State mutability can only be specified for address types.
// ParserError 9106: (938-942): State mutability can only be specified for address types.
// ParserError 9106: (960-964): State mutability can only be specified for address types.
// ParserError 9106: (984-988): State mutability can only be specified for address types.
// ParserError 9106: (1006-1010): State mutability can only be specified for address types.
// ParserError 9106: (1033-1037): State mutability can only be specified for address types.
// ParserError 9106: (1056-1060): State mutability can only be specified for address types.
// ParserError 9106: (1084-1088): State mutability can only be specified for address types.
