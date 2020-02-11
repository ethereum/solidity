contract Main {
    bytes3 name;
    bool flag;

    constructor(bytes3 x, bool f) public {
        name = x;
        flag = f;
    }

    function getName() public returns(bytes3 ret) {
        return name;
    }

    function getFlag() public returns(bool ret) {
        return flag;
    }
}

// ----
// getFlag() -> true
// getFlag():"" -> "1"
// getName() -> "abc"
// getName():"" -> "abc"
