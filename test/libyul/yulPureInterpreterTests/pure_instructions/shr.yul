{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }
    
    check(shr(0x0, 0x0), 0x0)
    check(shr(0x0, 0x1), 0x1)
    check(shr(0x0, 0x2), 0x2)
    check(shr(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(shr(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(shr(0x0, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0)
    check(shr(0x0, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb)
    check(shr(0x0, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de)
    check(shr(0x1, 0x0), 0x0)
    check(shr(0x1, 0x1), 0x0)
    check(shr(0x1, 0x2), 0x1)
    check(shr(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(shr(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(shr(0x1, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x5a47af7f111d77bb1a7a6583d1476c2b3f3cc7deab46e216f81e27c7e3f22ce8)
    check(shr(0x1, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x713f359d1c4bb8d3f7302983742ecfbc35e96e24141be8670e1e468da869f5f5)
    check(shr(0x1, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x28b8533510bca86d0177840526d76e1424734179948fcc171f4e8f1fe801586f)
    check(shr(0x2, 0x0), 0x0)
    check(shr(0x2, 0x1), 0x0)
    check(shr(0x2, 0x2), 0x0)
    check(shr(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(shr(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x3fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(shr(0x2, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x2d23d7bf888ebbdd8d3d32c1e8a3b6159f9e63ef55a3710b7c0f13e3f1f91674)
    check(shr(0x2, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x389f9ace8e25dc69fb9814c1ba1767de1af4b7120a0df433870f2346d434fafa)
    check(shr(0x2, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x145c299a885e543680bbc202936bb70a1239a0bcca47e60b8fa7478ff400ac37)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x0)
    check(shr(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x0)
    check(shr(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0x0), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0x1), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0x2), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x0)
    check(shr(0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0x0), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0x1), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0x2), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x0)
    check(shr(0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0x0), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0x1), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0x2), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0xb48f5efe223aef7634f4cb07a28ed8567e798fbd568dc42df03c4f8fc7e459d0), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0xe27e6b3a389771a7ee605306e85d9f786bd2dc482837d0ce1c3c8d1b50d3ebeb), 0x0)
    check(shr(0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de, 0x5170a66a217950da02ef080a4daedc2848e682f3291f982e3e9d1e3fd002b0de), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outter most variable values:
//
// Call trace:
