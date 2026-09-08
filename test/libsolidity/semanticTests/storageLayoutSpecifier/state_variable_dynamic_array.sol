contract A {
    uint[] public arrayA;
}

contract C is A layout at 42 {
    uint[] public arrayC;

    function initA() public returns (uint, uint, uint) {
        arrayA.push(1);
        arrayA.push(2);
        arrayA.push(3);

        return (arrayA[0], arrayA[1], arrayA[2]);
    }
    function initCFromAInReverse() public returns (uint, uint, uint) {
        arrayC = new uint[](3);
        arrayC[0] = arrayA[2];
        arrayC[1] = arrayA[1];
        arrayC[2] = arrayA[0];

        return (arrayC[0], arrayC[1], arrayC[2]);
    }
    function clearA () public {
        arrayA.pop();
        arrayA.pop();
        arrayA.pop();
    }
    function arrayALength() public returns (uint) {
        return arrayA.length;
    }
    function arrayCLength() public returns (uint) {
        return arrayC.length;
    }
}
// ----
// initA() -> 1, 2, 3
// gas irOptimized: 111983
// gas legacy: 111679
// gas legacyOptimized: 111150
// gas ssaCFGOptimized: 111964
// arrayA(uint256): 0 -> 1
// arrayALength() -> 3
// arrayCLength() -> 0
// initCFromAInReverse() -> 3, 2, 1
// gas irOptimized: 121278
// gas legacy: 121937
// gas legacyOptimized: 120853
// gas ssaCFGOptimized: 121220
// clearA() ->
// arrayC(uint256): 0 -> 3
// arrayALength() -> 0
// arrayCLength() -> 3
