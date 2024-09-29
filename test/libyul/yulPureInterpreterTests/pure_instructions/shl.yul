{

    function check(a, b)
    { if iszero(eq(a, b)) { revert(0, 0) } }
    
    check(shl(0x0, 0x0), 0x0)
    check(shl(0x0, 0x1), 0x1)
    check(shl(0x0, 0x2), 0x2)
    check(shl(0x0, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
    check(shl(0x0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(shl(0x0, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9)
    check(shl(0x0, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a)
    check(shl(0x0, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951)
    check(shl(0x1, 0x0), 0x0)
    check(shl(0x1, 0x1), 0x2)
    check(shl(0x1, 0x2), 0x4)
    check(shl(0x1, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe)
    check(shl(0x1, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    check(shl(0x1, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0xc5f180d19556f397126a43965fdc0465af1d2d7a32c3d3d4fd2e386d2f96fd2)
    check(shl(0x1, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0xc64b6e35896bfda7e82d9d3f3931aadbaf2221b8d6d0f1dddecff225c89556d4)
    check(shl(0x1, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0x61eeb35c944b39051996f7eb6d90f18dfcdf8b5aeddd2c1b58440be727f7b2a2)
    check(shl(0x2, 0x0), 0x0)
    check(shl(0x2, 0x1), 0x4)
    check(shl(0x2, 0x2), 0x8)
    check(shl(0x2, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc)
    check(shl(0x2, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff8)
    check(shl(0x2, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x18be301a32aade72e24d4872cbfb808cb5e3a5af46587a7a9fa5c70da5f2dfa4)
    check(shl(0x2, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x8c96dc6b12d7fb4fd05b3a7e726355b75e444371ada1e3bbbd9fe44b912aada8)
    check(shl(0x2, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0xc3dd66b92896720a332defd6db21e31bf9bf16b5dbba5836b08817ce4fef6544)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x0), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x1), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x2), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x0)
    check(shl(0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x0), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x1), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x2), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x0)
    check(shl(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0x0), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0x1), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0x2), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x0)
    check(shl(0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0x0), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0x1), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0x2), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x0)
    check(shl(0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0x0), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0x1), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0x2), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0x62f8c068caab79cb893521cb2fee0232d78e96bd1961e9ea7e971c3697cb7e9), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0x6325b71ac4b5fed3f416ce9f9c98d56dd79110dc6b6878eeef67f912e44aab6a), 0x0)
    check(shl(0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951, 0xb0f759ae4a259c828ccb7bf5b6c878c6fe6fc5ad76ee960dac2205f393fbd951), 0x0)
}
// ====
// maxTraceSize: 0
// ----
// Execution result: ExecutionOk
// Outter most variable values:
//
// Call trace:
