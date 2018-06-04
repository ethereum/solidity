// bug #1798 (cpp-ethereum), related to #1286 (solidity)
contract attribute {
    bool ok = false;
}
contract func {
    function ok() public returns (bool) { return true; }
}
contract attr_func is attribute, func {
    function checkOk() public returns (bool) { return ok(); }
}
// ----
// DeclarationError: (121-173): Identifier already declared.
