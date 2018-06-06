contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test() public {
        a = uint256(ActionChoices.GoStraight);
        b = uint64(ActionChoices.Sit);
    }
    uint256 a;
    uint64 b;
}
// ----
// Warning: (80-196): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
