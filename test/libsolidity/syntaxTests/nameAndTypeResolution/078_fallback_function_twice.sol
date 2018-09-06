contract C {
    uint x;
    function() external { x = 2; }
    function() external { x = 3; }
}
// ----
// DeclarationError: (64-94): Only one fallback function is allowed.
