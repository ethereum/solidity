{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(sar(0x0, 0x0), 0x0)
    check(sar(0x0, 0x1), 0x1)
    check(sar(0x0, 0x2), 0x2)
    check(sar(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(sar(0x0, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb)
    check(sar(0x0, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979)
    check(sar(0x0, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c)
    check(sar(0x1, 0x0), 0x0)
    check(sar(0x1, 0x1), 0x0)
    check(sar(0x1, 0x2), 0x1)
    check(sar(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x1, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xe4a4b0b5ab81acc1193f45fa3d9f041e96aa99e28af4d1a9c6e6ead742ce3465)
    check(sar(0x1, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x2ee4bc6eb8ef0d1c046a2da9c70e398fe0999af96de414d753adc956885e3cbc)
    check(sar(0x1, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x3e08e31677e058c595c611f22023507bfa4715f195e5e0a4acdcd494f86c46ce)
    check(sar(0x2, 0x0), 0x0)
    check(sar(0x2, 0x1), 0x0)
    check(sar(0x2, 0x2), 0x0)
    check(sar(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x2, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xf252585ad5c0d6608c9fa2fd1ecf820f4b554cf1457a68d4e373756ba1671a32)
    check(sar(0x2, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x17725e375c77868e023516d4e3871cc7f04ccd7cb6f20a6ba9d6e4ab442f1e5e)
    check(sar(0x2, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x1f04718b3bf02c62cae308f91011a83dfd238af8caf2f052566e6a4a7c362367)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0x0)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0x0)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x0)
    check(sar(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x0)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0x0)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0x0)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x0)
    check(sar(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x0)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0x0), 0x0)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0x1), 0x0)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0x2), 0x0)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x0)
    check(sar(0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x0)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0x0), 0x0)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0x1), 0x0)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0x2), 0x0)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x0)
    check(sar(0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x0)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0x0), 0x0)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0x1), 0x0)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0x2), 0x0)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0xc949616b57035982327e8bf47b3e083d2d5533c515e9a3538dcdd5ae859c68cb), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0x5dc978dd71de1a3808d45b538e1c731fc13335f2dbc829aea75b92ad10bc7979), 0x0)
    check(sar(0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c, 0x7c11c62cefc0b18b2b8c23e44046a0f7f48e2be32bcbc14959b9a929f0d88d9c), 0x0)
}
// ====
// EVMVersion: >=constantinople
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
