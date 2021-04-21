contract test {
    struct testStruct {
        uint256 m_value;
    }
    testStruct data1;

    constructor() {
        data1.m_value = 2;
    }

    function deleteMember() public returns (uint256 ret_value) {
        testStruct storage x = data1; //should not copy the data. data1.m_value == 2 but x.m_value = 0
        x.m_value = 4;
        delete x.m_value;
        ret_value = data1.m_value;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// deleteMember() -> 0
