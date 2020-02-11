// compileAndRunWithoutCheck(sourceCode, 0, "A" -> 
// 
contract A {
    uint public test = 1;
    uint[3] arr;
    constructor() public {
        uint index = 5;
        test = arr[index];
        ++test;
    }
}

// ----
-
> ""
