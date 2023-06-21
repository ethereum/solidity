object "root" {
  code {
      sstore(0, datasize("root"))
      sstore(1, datasize("0"))
      sstore(2, datasize("1"))
      sstore(3, datasize("2"))
      sstore(4, datasize("3"))
      sstore(5, datasize("4"))
      sstore(6, datasize("5"))
      sstore(7, datasize("6"))
      sstore(8, datasize("7"))
      sstore(9, datasize("8"))
      sstore(10, datasize("9"))
      sstore(11, datasize("a"))
      sstore(12, datasize("b"))
      sstore(13, datasize("c"))
      sstore(14, datasize("d"))
      sstore(15, datasize("e"))
      sstore(16, datasize("f"))
      sstore(17, datasize("10"))
  }

  object "0" {
    code {
      sstore(100, 0)
      sstore(200, datasize("sub0"))
    }
    object "sub0" {
      code {
        sstore(300, 0)
      }
    }
  }

  object "1" {
    code {
      sstore(100, 1)
    }
  }

  object "2" {
    code {
      sstore(101, 2)
    }
  }

  object "3" {
    code {
      sstore(102, 3)
    }
  }

  object "4" {
    code {
      sstore(103, 4)
    }
  }

  object "5" {
    code {
      sstore(104, 5)
    }
  }

  object "6" {
    code {
      sstore(105, 6)
    }
  }

  object "7" {
    code {
      sstore(106, 7)
    }
  }

  object "8" {
    code {
      sstore(107, 8)
    }
  }

  object "9" {
    code {
      sstore(108, 9)
    }
  }

  object "a" {
    code {
      sstore(109, 10)
    }
  }

  object "b" {
    code {
      sstore(110, 11)
    }
  }

  object "c" {
    code {
      sstore(111, 12)
    }
  }

  object "d" {
    code {
      sstore(112, 13)
    }
  }

  object "e" {
    code {
      sstore(113, 14)
    }
  }

  object "f" {
    code {
      sstore(114, 15)
    }
  }

  object "10" {
    code {
      sstore(115, 16)
      sstore(201, datasize("sub10"))
    }
    object "sub10" {
      code {
        sstore(300, 16)
      }
    }
  }
}
// ----
// Assembly:
//     /* "source":41:57   */
//   bytecodeSize
//     /* "source":38:39   */
//   0x00
//     /* "source":31:58   */
//   sstore
//     /* "source":75:88   */
//   dataSize(sub_0)
//     /* "source":72:73   */
//   0x01
//     /* "source":65:89   */
//   sstore
//     /* "source":106:119   */
//   dataSize(sub_1)
//     /* "source":103:104   */
//   0x02
//     /* "source":96:120   */
//   sstore
//     /* "source":137:150   */
//   dataSize(sub_2)
//     /* "source":134:135   */
//   0x03
//     /* "source":127:151   */
//   sstore
//     /* "source":168:181   */
//   dataSize(sub_3)
//     /* "source":165:166   */
//   0x04
//     /* "source":158:182   */
//   sstore
//     /* "source":199:212   */
//   dataSize(sub_4)
//     /* "source":196:197   */
//   0x05
//     /* "source":189:213   */
//   sstore
//     /* "source":230:243   */
//   dataSize(sub_5)
//     /* "source":227:228   */
//   0x06
//     /* "source":220:244   */
//   sstore
//     /* "source":261:274   */
//   dataSize(sub_6)
//     /* "source":258:259   */
//   0x07
//     /* "source":251:275   */
//   sstore
//     /* "source":292:305   */
//   dataSize(sub_7)
//     /* "source":289:290   */
//   0x08
//     /* "source":282:306   */
//   sstore
//     /* "source":323:336   */
//   dataSize(sub_8)
//     /* "source":320:321   */
//   0x09
//     /* "source":313:337   */
//   sstore
//     /* "source":355:368   */
//   dataSize(sub_9)
//     /* "source":351:353   */
//   0x0a
//     /* "source":344:369   */
//   sstore
//     /* "source":387:400   */
//   dataSize(sub_10)
//     /* "source":383:385   */
//   0x0b
//     /* "source":376:401   */
//   sstore
//     /* "source":419:432   */
//   dataSize(sub_11)
//     /* "source":415:417   */
//   0x0c
//     /* "source":408:433   */
//   sstore
//     /* "source":451:464   */
//   dataSize(sub_12)
//     /* "source":447:449   */
//   0x0d
//     /* "source":440:465   */
//   sstore
//     /* "source":483:496   */
//   dataSize(sub_13)
//     /* "source":479:481   */
//   0x0e
//     /* "source":472:497   */
//   sstore
//     /* "source":515:528   */
//   dataSize(sub_14)
//     /* "source":511:513   */
//   0x0f
//     /* "source":504:529   */
//   sstore
//     /* "source":547:560   */
//   dataSize(sub_15)
//     /* "source":543:545   */
//   0x10
//     /* "source":536:561   */
//   sstore
//     /* "source":579:593   */
//   dataSize(sub_16)
//     /* "source":575:577   */
//   0x11
//     /* "source":568:594   */
//   sstore
//     /* "source":23:598   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":644:645   */
//       0x00
//         /* "source":639:642   */
//       0x64
//         /* "source":632:646   */
//       sstore
//         /* "source":665:681   */
//       dataSize(sub_0)
//         /* "source":660:663   */
//       0xc8
//         /* "source":653:682   */
//       sstore
//         /* "source":624:688   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":742:743   */
//           0x00
//             /* "source":737:740   */
//           0x012c
//             /* "source":730:744   */
//           sstore
//             /* "source":720:752   */
//           stop
//     }
// }
//
// sub_1: assembly {
//         /* "source":808:809   */
//       0x01
//         /* "source":803:806   */
//       0x64
//         /* "source":796:810   */
//       sstore
//         /* "source":788:816   */
//       stop
// }
//
// sub_2: assembly {
//         /* "source":866:867   */
//       0x02
//         /* "source":861:864   */
//       0x65
//         /* "source":854:868   */
//       sstore
//         /* "source":846:874   */
//       stop
// }
//
// sub_3: assembly {
//         /* "source":924:925   */
//       0x03
//         /* "source":919:922   */
//       0x66
//         /* "source":912:926   */
//       sstore
//         /* "source":904:932   */
//       stop
// }
//
// sub_4: assembly {
//         /* "source":982:983   */
//       0x04
//         /* "source":977:980   */
//       0x67
//         /* "source":970:984   */
//       sstore
//         /* "source":962:990   */
//       stop
// }
//
// sub_5: assembly {
//         /* "source":1040:1041   */
//       0x05
//         /* "source":1035:1038   */
//       0x68
//         /* "source":1028:1042   */
//       sstore
//         /* "source":1020:1048   */
//       stop
// }
//
// sub_6: assembly {
//         /* "source":1098:1099   */
//       0x06
//         /* "source":1093:1096   */
//       0x69
//         /* "source":1086:1100   */
//       sstore
//         /* "source":1078:1106   */
//       stop
// }
//
// sub_7: assembly {
//         /* "source":1156:1157   */
//       0x07
//         /* "source":1151:1154   */
//       0x6a
//         /* "source":1144:1158   */
//       sstore
//         /* "source":1136:1164   */
//       stop
// }
//
// sub_8: assembly {
//         /* "source":1214:1215   */
//       0x08
//         /* "source":1209:1212   */
//       0x6b
//         /* "source":1202:1216   */
//       sstore
//         /* "source":1194:1222   */
//       stop
// }
//
// sub_9: assembly {
//         /* "source":1272:1273   */
//       0x09
//         /* "source":1267:1270   */
//       0x6c
//         /* "source":1260:1274   */
//       sstore
//         /* "source":1252:1280   */
//       stop
// }
//
// sub_10: assembly {
//         /* "source":1330:1332   */
//       0x0a
//         /* "source":1325:1328   */
//       0x6d
//         /* "source":1318:1333   */
//       sstore
//         /* "source":1310:1339   */
//       stop
// }
//
// sub_11: assembly {
//         /* "source":1389:1391   */
//       0x0b
//         /* "source":1384:1387   */
//       0x6e
//         /* "source":1377:1392   */
//       sstore
//         /* "source":1369:1398   */
//       stop
// }
//
// sub_12: assembly {
//         /* "source":1448:1450   */
//       0x0c
//         /* "source":1443:1446   */
//       0x6f
//         /* "source":1436:1451   */
//       sstore
//         /* "source":1428:1457   */
//       stop
// }
//
// sub_13: assembly {
//         /* "source":1507:1509   */
//       0x0d
//         /* "source":1502:1505   */
//       0x70
//         /* "source":1495:1510   */
//       sstore
//         /* "source":1487:1516   */
//       stop
// }
//
// sub_14: assembly {
//         /* "source":1566:1568   */
//       0x0e
//         /* "source":1561:1564   */
//       0x71
//         /* "source":1554:1569   */
//       sstore
//         /* "source":1546:1575   */
//       stop
// }
//
// sub_15: assembly {
//         /* "source":1625:1627   */
//       0x0f
//         /* "source":1620:1623   */
//       0x72
//         /* "source":1613:1628   */
//       sstore
//         /* "source":1605:1634   */
//       stop
// }
//
// sub_16: assembly {
//         /* "source":1685:1687   */
//       0x10
//         /* "source":1680:1683   */
//       0x73
//         /* "source":1673:1688   */
//       sstore
//         /* "source":1707:1724   */
//       dataSize(sub_0)
//         /* "source":1702:1705   */
//       0xc9
//         /* "source":1695:1725   */
//       sstore
//         /* "source":1665:1731   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":1786:1788   */
//           0x10
//             /* "source":1781:1784   */
//           0x012c
//             /* "source":1774:1789   */
//           sstore
//             /* "source":1764:1797   */
//           stop
//     }
// }
// Bytecode: 61005c5f55600b600155600660025560066003556006600455600660055560066006556006600755600660085560066009556006600a556006600b556006600c556006600d556006600e556006600f556006601055600c60115500fe
// Opcodes: PUSH2 0x5C PUSH0 SSTORE PUSH1 0xB PUSH1 0x1 SSTORE PUSH1 0x6 PUSH1 0x2 SSTORE PUSH1 0x6 PUSH1 0x3 SSTORE PUSH1 0x6 PUSH1 0x4 SSTORE PUSH1 0x6 PUSH1 0x5 SSTORE PUSH1 0x6 PUSH1 0x6 SSTORE PUSH1 0x6 PUSH1 0x7 SSTORE PUSH1 0x6 PUSH1 0x8 SSTORE PUSH1 0x6 PUSH1 0x9 SSTORE PUSH1 0x6 PUSH1 0xA SSTORE PUSH1 0x6 PUSH1 0xB SSTORE PUSH1 0x6 PUSH1 0xC SSTORE PUSH1 0x6 PUSH1 0xD SSTORE PUSH1 0x6 PUSH1 0xE SSTORE PUSH1 0x6 PUSH1 0xF SSTORE PUSH1 0x6 PUSH1 0x10 SSTORE PUSH1 0xC PUSH1 0x11 SSTORE STOP INVALID
// SourceMappings: 41:16:0:-:0;38:1;31:27;75:13;72:1;65:24;106:13;103:1;96:24;137:13;134:1;127:24;168:13;165:1;158:24;199:13;196:1;189:24;230:13;227:1;220:24;261:13;258:1;251:24;292:13;289:1;282:24;323:13;320:1;313:24;355:13;351:2;344:25;387:13;383:2;376:25;419:13;415:2;408:25;451:13;447:2;440:25;483:13;479:2;472:25;515:13;511:2;504:25;547:13;543:2;536:25;579:14;575:2;568:26;23:575
