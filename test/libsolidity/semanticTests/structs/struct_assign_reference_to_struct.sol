contract test {
    struct testStruct {
        uint256 m_value;
    }
    testStruct data1;
    testStruct data2;
    testStruct data3;

    constructor() {
        data1.m_value = 2;
    }

    function assign()
        public
        returns (
            uint256 ret_local,
            uint256 ret_global,
            uint256 ret_global3,
            uint256 ret_global1
        )
    {
        testStruct storage x = data1; //x is a reference data1.m_value == 2 as well as x.m_value = 2
        data2 = data1; // should copy data. data2.m_value == 2

        ret_local = x.m_value; // = 2
        ret_global = data2.m_value; // = 2

        x.m_value = 3;
        data3 = x; //should copy the data. data3.m_value == 3
        ret_global3 = data3.m_value; // = 3
        ret_global1 = data1.m_value; // = 3. Changed due to the assignment to x.m_value
    }
}
// ----
// assign() -> 2, 2, 3, 3
