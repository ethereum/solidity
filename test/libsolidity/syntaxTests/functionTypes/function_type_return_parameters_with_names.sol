contract C {
    function(uint) returns (bool ret) f;
}
// ----
// SyntaxError: (41-49): Return parameters in function types may not be named.
