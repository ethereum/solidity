contract test {
    struct testStruct {
        uint m_value;
    }
    mapping(uint => testStruct) campaigns;

    constructor() public {
        campaigns[0].m_value = 2;
    }

    function deleteIt() public returns(uint) {
        delete campaigns[0];
        return campaigns[0].m_value;
    }
}

// ----
// deleteIt() -> 0
// deleteIt():"" -> "0"
