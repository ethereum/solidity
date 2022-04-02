contract C {
    mapping(uint=>uint)[] array;
    mapping(uint=>uint) map;
    function f() public {
        array.push();
        array.push(map);
    }
}
// ----
// TypeError 8871: (131-141='array.push'): Storage arrays with nested mappings do not support .push(<arg>).
