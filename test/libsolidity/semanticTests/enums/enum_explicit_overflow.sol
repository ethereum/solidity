contract test {
    enum ActionChoices {GoLeft, GoRight, GoStraight}

    constructor() {}

    function getChoiceExp(uint256 x) public returns (uint256 d) {
        choice = ActionChoices(x);
        d = uint256(choice);
    }

    function getChoiceFromSigned(int256 x) public returns (uint256 d) {
        choice = ActionChoices(x);
        d = uint256(choice);
    }

    function getChoiceFromNegativeLiteral() public returns (uint256 d) {
        choice = ActionChoices(-1);
        d = uint256(choice);
    }

    ActionChoices choice;
}

// ====
// compileViaYul: also
// ----
// getChoiceExp(uint256): 3 -> FAILURE # These should throw #
// getChoiceFromSigned(int256): -1 -> FAILURE
// getChoiceFromNegativeLiteral() -> FAILURE
// getChoiceExp(uint256): 2 -> 2 # These should work #
// getChoiceExp(uint256): 0 -> 0
