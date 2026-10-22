contract C {
    struct S {
        function() f;
    }
    function g(S[2] calldata) internal pure {}
}
// ----
