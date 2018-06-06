contract C {
    function f() public {
        mapping(uint => uint) x;
        x;
    }
}
// ----
// TypeError: (47-70): Uninitialized mapping. Mappings cannot be created dynamically, you have to assign them from a state variable.
