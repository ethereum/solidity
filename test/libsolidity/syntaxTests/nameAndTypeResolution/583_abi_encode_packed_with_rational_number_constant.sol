contract C {
    function f() pure public { abi.encodePacked(0/1); }
}
// ----
// TypeError: (61-64): Cannot perform packed encoding for a literal. Please convert it to an explicit type first.