contract C {
    function f() pure public { selfdestruct; }
}
// ----
// Warning 5159: (44-56): "selfdestruct" has been deprecated. The underlying opcode will eventually undergo breaking changes, and its use is not recommended.
// Warning 6133: (44-56): Statement has no effect.
