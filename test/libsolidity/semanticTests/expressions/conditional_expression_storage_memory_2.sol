contract test {
    bytes2[2] data1;
    function f(bool cond) public returns (uint) {
        data1[0] = "cc";

        bytes2[2] memory x;
        bytes2[2] memory y;
        y[0] = "bb";

        x = cond ? y : data1;

        uint ret = 0;
        if (x[0] == "bb")
        {
            ret = 1;
        }

        if (x[0] == "cc")
        {
            ret = 2;
        }

        return ret;
    }
}
// ----
// f(bool): true -> 1
// f(bool): false -> 2
