contract test {
    enum ActionChoices {GoLeft, GoRight, GoStraight, Sit}

    constructor() {
        choices = ActionChoices.GoStraight;
    }

    function getChoice() public returns (uint256 d) {
        d = uint256(choices);
    }

    ActionChoices choices;
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// getChoice() -> 2
