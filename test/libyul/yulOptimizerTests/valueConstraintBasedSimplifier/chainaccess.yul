{
    // restricted to 160 bits
    let a := address()
    let b := origin()
    let c := caller()
    let d := coinbase()
    let e := create(0, 0, 0)
    let f := create2(0, 0, 0, 0)
    // restricted to bool
    let gb := call(0, 0, 0, 0, 0, 0, 0)
    let hb := callcode(0, 0, 0, 0, 0, 0, 0)
    let ib := delegatecall(0, 0, 0, 0, 0, 0)
    let jb := staticcall(0, 0, 0, 0, 0, 0)
    // No restriction starting from here
    let k_ := keccak256(0, 0)
    let l_ := extcodehash(0)
    let m_ := blockhash(0)
    let n_ := mload(0)
    let o_ := sload(0)
    let p_ := balance(0)
    let q_ := callvalue()
    let r_ := calldatasize()
    let s_ := gasprice()
    let t_ := extcodesize(0)
    let u_ := returndatasize()
    let v_ := timestamp()
    let w_ := number()
    let x_ := difficulty()
    let y_ := gaslimit()
    let z_ := pc()
    let z1_ := msize()
    let z2_ := gas()
}
// ----
// valueConstraintBasedSimplifier
// a:
//     min: 0
//     max: 2**160 - 1
//    minB: 0
//    maxB: 2**160 - 1
// b:
//     min: 0
//     max: 2**160 - 1
//    minB: 0
//    maxB: 2**160 - 1
// c:
//     min: 0
//     max: 2**160 - 1
//    minB: 0
//    maxB: 2**160 - 1
// d:
//     min: 0
//     max: 2**160 - 1
//    minB: 0
//    maxB: 2**160 - 1
// e:
//     min: 0
//     max: 2**160 - 1
//    minB: 0
//    maxB: 2**160 - 1
// f:
//     min: 0
//     max: 2**160 - 1
//    minB: 0
//    maxB: 2**160 - 1
// gb:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// hb:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// ib:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// jb:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// k_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// l_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// m_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// n_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// o_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// p_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// q_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// r_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// s_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// t_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// u_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// v_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// w_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// x_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// y_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// z_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// z1_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// z2_:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// {
//     let a := address()
//     let b := origin()
//     let c := caller()
//     let d := coinbase()
//     let e := create(0, 0, 0)
//     let f := create2(0, 0, 0, 0)
//     let gb := call(0, 0, 0, 0, 0, 0, 0)
//     let hb := callcode(0, 0, 0, 0, 0, 0, 0)
//     let ib := delegatecall(0, 0, 0, 0, 0, 0)
//     let jb := staticcall(0, 0, 0, 0, 0, 0)
//     let k_ := keccak256(0, 0)
//     let l_ := extcodehash(0)
//     let m_ := blockhash(0)
//     let n_ := mload(0)
//     let o_ := sload(0)
//     let p_ := balance(0)
//     let q_ := callvalue()
//     let r_ := calldatasize()
//     let s_ := gasprice()
//     let t_ := extcodesize(0)
//     let u_ := returndatasize()
//     let v_ := timestamp()
//     let w_ := number()
//     let x_ := difficulty()
//     let y_ := gaslimit()
//     let z_ := pc()
//     let z1_ := msize()
//     let z2_ := gas()
// }
