contract A {
    uint[] x;
    function f() public view returns (uint) {
        return x.push();
    }
}
// ----
// TypeError: (88-96): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
