contract C {
    function a() public pure {
        try this.a() {} catch (string payable memory) {}
    }
    function c() public pure {
        try this.c() {} catch (bytes payable memory) {}
    }
}
contract C2 {
    function a() public pure {
        try this.a() {} catch (string view memory) {}
    }
    function c() public pure {
        try this.c() {} catch (bytes view memory) {}
    }
}
contract C3 {
    function a() public pure {
        try this.a() {} catch (string pure memory) {}
    }
    function c() public pure {
        try this.c() {} catch (bytes pure memory) {}
    }
}
// ----
// ParserError 9106: (82-89): State mutability can only be specified for address types.
// ParserError 9106: (175-182): State mutability can only be specified for address types.
// ParserError 9106: (285-289): State mutability can only be specified for address types.
// ParserError 9106: (375-379): State mutability can only be specified for address types.
// ParserError 9106: (482-486): State mutability can only be specified for address types.
// ParserError 9106: (572-576): State mutability can only be specified for address types.