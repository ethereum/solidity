contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() public {
        b = ActionChoices.Sit;
    }
    uint64 b;
}
// ----
// TypeError: (115-132): Type enum test.ActionChoices is not implicitly convertible to expected type uint64.
