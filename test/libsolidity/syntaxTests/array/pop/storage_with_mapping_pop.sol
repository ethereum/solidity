contract C {
    mapping(uint=>uint)[] array;
    mapping(uint=>uint) map;
    function f() public {
        array.pop();
    }
}
// ----
// TypeError 6298: (109-118): Storage arrays with nested mappings do not support .pop().
