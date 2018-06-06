contract Test {
    function f() public { var x = new Test(); }
}
// ----
// Warning: (42-47): Use of the "var" keyword is deprecated.
// TypeError: (50-58): Circular reference for contract creation (cannot create instance of derived or same contract).
