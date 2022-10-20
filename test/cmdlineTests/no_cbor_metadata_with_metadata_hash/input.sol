// SPDX-License-Identifier: GPL-3.0
pragma solidity >= 0.0.0;

contract Empty {}

contract C {
    function f() external returns (bytes memory, bytes memory){
        return (type(Empty).creationCode, type(Empty).runtimeCode);
    }
}
