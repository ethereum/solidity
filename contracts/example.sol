// SPDX-License-Identifier: MIT
pragma solidity >0.8.25;

library Lib {
    function set(mapping(address => uint256) transient m, uint256 v) internal { m[msg.sender] += v; }
    function set(mapping(address => uint256) storage   m, uint256 v) internal { m[msg.sender] += v; }
}

contract Test {
    using Lib for mapping(address => uint256);

    struct MyStruct {
        uint256 v;
        mapping(address => uint256) m;
    }

    bytes                                           transient /*public*/ b_t;
    bytes                                                     /*public*/ b_s;
    uint256                                         transient public v_t;
    uint256                                                   public v_s;
    uint256[]                                       transient public a_t;
    uint256[]                                                 public a_s;
    MyStruct                                        transient public s_t;
    MyStruct                                                  public s_s;
    mapping(address => uint256)                     transient public m_t;
    mapping(address => uint256)                               public m_s;
    mapping(uint256 => mapping(address => uint256)) transient public m2_t;
    mapping(uint256 => mapping(address => uint256))           public m2_s;

    function id(mapping(address => uint256) transient m) internal pure returns (mapping(address => uint256) transient) { return m; }
    function id(mapping(address => uint256) storage   m) internal pure returns (mapping(address => uint256) storage  ) { return m; }
    function set(mapping(address => uint256) transient m, uint256 v) internal { m[msg.sender] = v; }
    function set(mapping(address => uint256) storage   m, uint256 v) internal { m[msg.sender] = v; }

    function set(uint256 value) public {
        b_t[0] = 0xFF;
        b_s[0] = 0xFF;
        v_t += value;
        v_s += value;
        a_t.push(value);
        a_s.push(value);
        a_t[0] += value;
        a_s[0] += value;
        s_t.v += value;
        s_s.v += value;
        s_t.m.set(value);
        s_s.m.set(value);
        // m_t.set(value);
        // m_s.set(value);
        id(m_t).set(value);
        id(m_s).set(value);
        m2_t[0].set(value);
        m2_s[0].set(value);


        MyStruct transient s_t_ref = s_t;
        s_t_ref.v += value;

        MyStruct transient s;
        assembly { s.slot := 0 }
        s.v *= value;

        assembly {
            tstore(0, add(tload(0), value))
        }
    }
}
