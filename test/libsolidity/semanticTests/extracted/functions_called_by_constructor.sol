contract Test {
    bytes3 name;
    bool flag;

    constructor() public {
        setName("abc");
    }

    function getName() public returns (bytes3 ret) {
        return name;
    }

    function setName(bytes3 _name) private {
        name = _name;
    }
}

// ====
// compileViaYul: also
// ----
// getName() -> "abc"
