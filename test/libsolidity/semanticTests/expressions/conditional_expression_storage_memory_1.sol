contract test {
    bytes2[2] data1;
    function f(bool cond) public returns (uint) {
        bytes2[2] memory x;
        x[0] = "aa";
        bytes2[2] memory y;
        y[0] = "bb";

        data1 = cond ? x : y;

        uint ret = 0;
        if (data1[0] == "aa")
        {
            ret = 1;
        }

        if (data1[0] == "bb")
        {
            ret = 2;
        }

        return ret;
    }
}
// ----
// f(bool): true -> 1
// f(bool): false -> 2
