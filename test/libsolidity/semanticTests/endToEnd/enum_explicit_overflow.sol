contract test {
    enum ActionChoices {
        GoLeft,
        GoRight,
        GoStraight
    }
    constructor() public {}

    function getChoiceExp(uint x) public returns(uint d) {
        choice = ActionChoices(x);
        d = uint256(choice);
    }

    function getChoiceFromSigned(int x) public returns(uint d) {
        choice = ActionChoices(x);
        d = uint256(choice);
    }

    function getChoiceFromNegativeLiteral() public returns(uint d) {
        choice = ActionChoices(-1);
        d = uint256(choice);
    }
    ActionChoices choice;
}

// ====
// compileViaYul: also
// ----
// getChoiceExp(uint256): 3 -> 
// getChoiceExp(uint256):"3" -> ""
// getChoiceFromSigned(int256): -1 -> 
// getChoiceFromSigned(int256):"-1" -> ""
// getChoiceFromNegativeLiteral() -> 
// getChoiceFromNegativeLiteral():"" -> ""
// getChoiceExp(uint256): 2 -> 2
// getChoiceExp(uint256):"2" -> "2"
// getChoiceExp(uint256): 0 -> 0
// getChoiceExp(uint256):"0" -> "0"
