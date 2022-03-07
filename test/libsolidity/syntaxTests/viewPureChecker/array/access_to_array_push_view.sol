contract A {
    uint[] x;
    function f() public view returns (uint) {
        return x.push();
    }
}
// ----
// TypeError 8961: (88-96): Function cannot be declared as view because this expression (potentially) modifies the state.
