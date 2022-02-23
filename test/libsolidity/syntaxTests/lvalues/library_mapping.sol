library L {
    function f(mapping(uint=>uint) storage x, mapping(uint=>uint) storage y) external {
        x = y;
    }
}
// ----
// TypeError 9214: (108-109): Types in storage containing (nested) mappings cannot be assigned to.
