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

    function getChoiceFromMax() public returns (uint256 d) {
        choice = ActionChoices(type(uint256).max);
        d = uint256(choice);
    }

    ActionChoices choice;
}

// ====
// EVMVersion: <byzantium
// ----
// getChoiceExp(uint256): 3 -> FAILURE # These should throw #
// getChoiceFromSigned(int256): -1 -> FAILURE
// getChoiceFromMax() -> FAILURE
// getChoiceExp(uint256): 2 -> 2 # These should work #
// getChoiceExp(uint256): 0 -> 0
