library L {
    function f(mapping(uint=>uint) storage x, mapping(uint=>uint) storage y) external {
        x = y;
    }
}
// ----
// TypeError 9214: (108-109): Mappings cannot be assigned to.
