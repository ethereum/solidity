{
    // The optimizer assumes verbatim could contain msize,
    // so it cannot optimize the mload away.
    let x := mload(0x2000)
    verbatim_0i_0o("aa")
    sstore(0, 2)
}
