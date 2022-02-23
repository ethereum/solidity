contract test {
    enum ActionChoices { GoLeft, GoRight, GoStraight, Sit }
    constructor() {
        a = uint256(ActionChoices.GoStraight);
        b = uint64(ActionChoices.Sit);
    }
    uint256 a;
    uint64 b;
}
// ----
