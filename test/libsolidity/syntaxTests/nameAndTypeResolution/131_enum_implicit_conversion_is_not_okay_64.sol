contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    function test() public {
        b = ActionChoices.Sit;
    }
    uint64 b;
}
// ----
// Warning: (80-141): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// TypeError: (117-134): Type enum test.ActionChoices is not implicitly convertible to expected type uint64.
