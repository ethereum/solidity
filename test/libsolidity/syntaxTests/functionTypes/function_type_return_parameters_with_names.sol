contract C {
    function(uint) returns (bool ret) f;
}
// ----
// SyntaxError 7304: (41-49='bool ret'): Return parameters in function types may not be named.
