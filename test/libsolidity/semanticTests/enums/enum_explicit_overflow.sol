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
        choice = ActionChoices(type(uint).max);
        d = uint256(choice);
    }

    ActionChoices choice;
}

// ====
// compileViaYul: also
// EVMVersion: >=byzantium
// ----
// getChoiceExp(uint256): 2 -> 2
// getChoiceExp(uint256): 3 -> FAILURE, hex"4e487b71", 33 # These should throw #
// getChoiceFromSigned(int256): -1 -> FAILURE, hex"4e487b71", 33
// getChoiceFromMax() -> FAILURE, hex"4e487b71", 33
// getChoiceExp(uint256): 2 -> 2 # These should work #
// getChoiceExp(uint256): 0 -> 0
