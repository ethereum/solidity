contract C {

    function len() public returns (uint ret) {
        uint[] memory data = new uint[](2);
        data[0] = 234;
        data[1] = 123;
        delete data;
        assembly {
            ret := mload(data)
        }
    }
}

// ----
// len() -> 0
