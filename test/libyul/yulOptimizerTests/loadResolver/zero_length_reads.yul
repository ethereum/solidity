{
  returndatacopy(1, 1, 0)
  calldatacopy(1, 1, 0)
  extcodecopy(1, 1, 1, 0)
  codecopy(1, 1, 0)
  log0(1, 0)
  log1(1, 0, 1)
  log2(1, 0, 1, 1)
  log3(1, 0, 1, 1, 1)
  log4(1, 0, 1, 1, 1, 1)
  pop(create(1, 1, 0))
  pop(create2(1, 1, 0, 1))
  pop(call(1, 1, 1, 1, 0, 1, 0))
  pop(callcode(1, 1, 1, 1, 0, 1, 0))
  pop(delegatecall(1, 1, 1, 0, 1, 0))
  pop(staticcall(1, 1, 1, 0, 1, 0))
  return(1, 0)
}
// ====
// EVMVersion: >=constantinople
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let _2 := 1
//         returndatacopy(0, _2, _1)
//         calldatacopy(0, _2, _1)
//         extcodecopy(_2, 0, _2, _1)
//         codecopy(0, _2, _1)
//         log0(0, _1)
//         log1(0, _1, _2)
//         log2(0, _1, _2, _2)
//         log3(0, _1, _2, _2, _2)
//         log4(0, _1, _2, _2, _2, _2)
//         pop(create(_2, 0, _1))
//         pop(create2(_2, 0, _1, _2))
//         pop(call(_2, _2, _2, 0, _1, _2, _1))
//         pop(callcode(_2, _2, _2, 0, _1, _2, _1))
//         pop(delegatecall(_2, _2, 0, _1, _2, _1))
//         pop(staticcall(_2, _2, 0, _1, _2, _1))
//         return(0, _1)
//     }
// }
