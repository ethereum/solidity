// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

contract Test {
    struct MyStruct { uint256 value; }

    uint256                     transient a_t;
    uint256                               a_s;
    MyStruct                    transient s_t;
    MyStruct                              s_s;
    mapping(address => uint256) transient m_t;
    mapping(address => uint256)           m_s;

    function set(uint256 value) public {
        // a_t = value;
        // a_s = value;
        // s_t.value = value;
        // s_s.value = value;
        m_t[msg.sender] = value;
        // m_s[msg.sender] = value;
    }
/*
    function get() public view returns (uint256 value) {
        // MyStruct transient s;
        // assembly { s.slot := 0 }
        // return s.value;
        return 1;
    }
*/
}
