contract C {
    uint x;
    function() public { x = 2; }
    function() public { x = 3; }
}
// ----
// DeclarationError: (62-90): Only one fallback function is allowed.
