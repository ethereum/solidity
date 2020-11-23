// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.0;
contract test {
    function f() public pure returns (string memory) {
        return type(test).name;
    }
}