contract c {
    bytes data;

    function test() public returns(bytes memory) {
        for (uint i = 0; i < 34; i++)
            data.push(0x03);
        data.pop();
        return data;
    }
}

// ----
// test() -> 32, 33, 0x0303030303030303030303030303030303030303030303030303030303030303, 1356938545749799165119972480570561420155507632800475359837393562592731987968
