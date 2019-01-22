contract Test {
    function f() public pure returns (uint) {
        return type(C).runtimeCode.length +
            type(D).runtimeCode.length +
            type(C).creationCode.length +
            type(D).creationCode.length;
    }
}
contract C {
    constructor() public { assembly {} }
}
contract D is C {
    constructor() public {}
}
// ----
// Warning: (77-96): The constructor of the contract (or its base) uses inline assembly. Because of that, it might be that the deployed bytecode is different from type(...).runtimeCode.
// Warning: (118-137): The constructor of the contract (or its base) uses inline assembly. Because of that, it might be that the deployed bytecode is different from type(...).runtimeCode.
