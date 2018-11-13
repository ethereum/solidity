contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() public {
        a = 2;
        b = ActionChoices(a);
    }
    uint256 a;
    ActionChoices b;
}
// ----
