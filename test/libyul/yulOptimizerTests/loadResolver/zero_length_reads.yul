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
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 1
//         returndatacopy(0, _1, 0)
//         calldatacopy(0, _1, 0)
//         extcodecopy(_1, 0, _1, 0)
//         codecopy(0, _1, 0)
//         log0(0, 0)
//         log1(0, 0, _1)
//         log2(0, 0, _1, _1)
//         log3(0, 0, _1, _1, _1)
//         log4(0, 0, _1, _1, _1, _1)
//         pop(create(_1, 0, 0))
//         pop(create2(_1, 0, 0, _1))
//         pop(call(_1, _1, _1, 0, 0, _1, 0))
//         pop(callcode(_1, _1, _1, 0, 0, _1, 0))
//         pop(delegatecall(_1, _1, 0, 0, _1, 0))
//         pop(staticcall(_1, _1, 0, 0, _1, 0))
//         return(0, 0)
//     }
// }
