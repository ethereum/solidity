pragma solidity >= 0.6.0;

contract C {
    function d(uint n) external pure returns (uint[] memory) {
        uint[] memory data = new uint[](n);
        for (uint i = 0; i < data.length; ++i)
            data[i] = i;
        return data;
    }

    function dt(uint n) public view returns (uint) {
        uint[] memory data = this.d(n);
        uint sum = 0;
        for (uint i = 0; i < data.length; ++i)
            sum += data[i];
        return sum;
    }
}

// ====
// EVMVersion: >=byzantium
// ----
// dt(uint256): 4 -> 6
