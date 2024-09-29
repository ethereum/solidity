{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }

    check(lt(0x0, 0x0), 0x0)
    check(lt(0x0, 0x1), 0x1)
    check(lt(0x0, 0x2), 0x1)
    check(lt(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(lt(0x0, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x1)
    check(lt(0x0, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x1)
    check(lt(0x0, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x1)
    check(lt(0x1, 0x0), 0x0)
    check(lt(0x1, 0x1), 0x0)
    check(lt(0x1, 0x2), 0x1)
    check(lt(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(lt(0x1, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x1)
    check(lt(0x1, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x1)
    check(lt(0x1, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x1)
    check(lt(0x2, 0x0), 0x0)
    check(lt(0x2, 0x1), 0x0)
    check(lt(0x2, 0x2), 0x0)
    check(lt(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(lt(0x2, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x1)
    check(lt(0x2, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x1)
    check(lt(0x2, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x1)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x0)
    check(lt(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x0)
    check(lt(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x0)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0x0), 0x0)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0x1), 0x0)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0x2), 0x0)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x0)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x0)
    check(lt(0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x0)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0x0), 0x0)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0x1), 0x0)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0x2), 0x0)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x1)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x0)
    check(lt(0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x1)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0x0), 0x0)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0x1), 0x0)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0x2), 0x0)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x1)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x1)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0xae19640f021de5090eeadd850aeeae859a06c5d88963f273287de022ca85d145), 0x1)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0x152157ed731b1ce9eb9bc7be7ab3990a6a35c553715200196b0c17c558ed0658), 0x0)
    check(lt(0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b, 0x2a8f6ec8110a734f8851187a986caa2020da16c031e270b19d0de0edbacacb3b), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outer most variable values:
//
// Call trace:
