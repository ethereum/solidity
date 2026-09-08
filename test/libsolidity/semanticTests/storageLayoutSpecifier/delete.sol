contract C layout at 42 {
    uint[] public array;

    function fillArray() public returns (uint) {
        array.push(1);
        array.push(2);
        array.push(3);
        return array.length;
    }
    function deleteLast() public {
        delete array[array.length - 1];
    }
    function deleteArray() public {
        delete array;
    }
    function arrayLength() public returns (uint) {
        return array.length;
    }
}
// ----
// fillArray() -> 3
// gas irOptimized: 111118
// gas legacy: 110730
// gas legacyOptimized: 110307
// gas ssaCFGOptimized: 111129
// arrayLength() -> 3
// array(uint256): 2 -> 3
// deleteLast() ->
// array(uint256): 2 -> 0
// deleteArray() ->
// arrayLength() -> 0
