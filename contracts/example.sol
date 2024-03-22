// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

contract Test {
    struct MyStruct { uint256 value; }

    uint256                                         transient v_t;
    uint256                                                   v_s;
    uint256[]                                       transient a_t;
    uint256[]                                                 a_s;
    MyStruct                                        transient s_t;
    MyStruct                                                  s_s;
    mapping(address => uint256)                     transient m_t;
    mapping(address => uint256)                               m_s;
    mapping(address => mapping(uint256 => uint256)) transient m2_t;
    mapping(address => mapping(uint256 => uint256))           m2_s;

    function set(uint256 value) public {
        v_t = value;
        v_s = value;
        // a_t.push(value); // Error: Member "push" is not available in uint256[] transient pointer outside of storage.
        a_s.push(value);
        a_t[0] = value;
        a_s[0] = value;
        s_t.value = value;
        s_s.value = value;
        m_t[msg.sender] = value;
        m_s[msg.sender] = value;
        m2_t[msg.sender][0] = value;
        m2_s[msg.sender][0] = value;

        // MyStruct transient s;
        // assembly { s.slot := 0 } // Error: The suffix ".slot" is not supported by this variable or type.
        // s.value = value;
    }
}
