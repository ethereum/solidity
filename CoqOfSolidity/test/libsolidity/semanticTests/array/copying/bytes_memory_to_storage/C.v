(* Generated by coq-of-solidity *)
Require Import CoqOfSolidity.CoqOfSolidity.

Module C.
  Definition code : Code.t := {|
    Code.name := "C_21";
    Code.hex_name := 0x435f323100000000000000000000000000000000000000000000000000000000;
    Code.functions :=
      [
        
      ];
    Code.body :=
      M.scope (
        do! ltac:(M.monadic (
          M.scope (
            do! ltac:(M.monadic (
              M.declare (|
                ["_1"],
                Some (M.call_function (|
                  "memoryguard",
                  [
                    [Literal.number 0x80]
                  ]
                |))
              |)
            )) in
            do! ltac:(M.monadic (
              M.expr_stmt (|
                M.call_function (|
                  "mstore",
                  [
                    [Literal.number 64];
                    M.get_var (| "_1" |)
                  ]
                |)
              |)
            )) in
            do! ltac:(M.monadic (
              M.if_ (|
                M.call_function (|
                  "callvalue",
                  []
                |),
                M.scope (
                  do! ltac:(M.monadic (
                    M.expr_stmt (|
                      M.call_function (|
                        "revert",
                        [
                          [Literal.number 0];
                          [Literal.number 0]
                        ]
                      |)
                    |)
                  )) in
                  M.pure BlockUnit.Tt
                )
              |)
            )) in
            do! ltac:(M.monadic (
              M.declare (|
                ["_2"],
                Some (M.call_function (|
                  "datasize",
                  [
                    [Literal.string 0x435f32315f6465706c6f79656400000000000000000000000000000000000000]
                  ]
                |))
              |)
            )) in
            do! ltac:(M.monadic (
              M.expr_stmt (|
                M.call_function (|
                  "codecopy",
                  [
                    M.get_var (| "_1" |);
                    M.call_function (|
                      "dataoffset",
                      [
                        [Literal.string 0x435f32315f6465706c6f79656400000000000000000000000000000000000000]
                      ]
                    |);
                    M.get_var (| "_2" |)
                  ]
                |)
              |)
            )) in
            do! ltac:(M.monadic (
              M.expr_stmt (|
                M.call_function (|
                  "return",
                  [
                    M.get_var (| "_1" |);
                    M.get_var (| "_2" |)
                  ]
                |)
              |)
            )) in
            M.pure BlockUnit.Tt
          )
        )) in
        M.pure BlockUnit.Tt
      );
  |}.

  Module deployed.
    Definition code : Code.t := {|
      Code.name := "C_21_deployed";
      Code.hex_name := 0x435f32315f6465706c6f79656400000000000000000000000000000000000000;
      Code.functions :=
        [
          Code.Function.make (
            "extract_byte_array_length",
            ["data"],
            ["length"],
            M.scope (
              do! ltac:(M.monadic (
                M.assign (|
                  ["length"],
                  Some (M.call_function (|
                    "shr",
                    [
                      [Literal.number 1];
                      M.get_var (| "data" |)
                    ]
                  |))
                |)
              )) in
              do! ltac:(M.monadic (
                M.declare (|
                  ["outOfPlaceEncoding"],
                  Some (M.call_function (|
                    "and",
                    [
                      M.get_var (| "data" |);
                      [Literal.number 1]
                    ]
                  |))
                |)
              )) in
              do! ltac:(M.monadic (
                M.if_ (|
                  M.call_function (|
                    "iszero",
                    [
                      M.get_var (| "outOfPlaceEncoding" |)
                    ]
                  |),
                  M.scope (
                    do! ltac:(M.monadic (
                      M.assign (|
                        ["length"],
                        Some (M.call_function (|
                          "and",
                          [
                            M.get_var (| "length" |);
                            [Literal.number 0x7f]
                          ]
                        |))
                      |)
                    )) in
                    M.pure BlockUnit.Tt
                  )
                |)
              )) in
              do! ltac:(M.monadic (
                M.if_ (|
                  M.call_function (|
                    "eq",
                    [
                      M.get_var (| "outOfPlaceEncoding" |);
                      M.call_function (|
                        "lt",
                        [
                          M.get_var (| "length" |);
                          [Literal.number 32]
                        ]
                      |)
                    ]
                  |),
                  M.scope (
                    do! ltac:(M.monadic (
                      M.expr_stmt (|
                        M.call_function (|
                          "mstore",
                          [
                            [Literal.number 0];
                            M.call_function (|
                              "shl",
                              [
                                [Literal.number 224];
                                [Literal.number 0x4e487b71]
                              ]
                            |)
                          ]
                        |)
                      |)
                    )) in
                    do! ltac:(M.monadic (
                      M.expr_stmt (|
                        M.call_function (|
                          "mstore",
                          [
                            [Literal.number 4];
                            [Literal.number 0x22]
                          ]
                        |)
                      |)
                    )) in
                    do! ltac:(M.monadic (
                      M.expr_stmt (|
                        M.call_function (|
                          "revert",
                          [
                            [Literal.number 0];
                            [Literal.number 0x24]
                          ]
                        |)
                      |)
                    )) in
                    M.pure BlockUnit.Tt
                  )
                |)
              )) in
              M.pure BlockUnit.Tt
            )
          )
        ];
      Code.body :=
        M.scope (
          do! ltac:(M.monadic (
            M.scope (
              do! ltac:(M.monadic (
                M.declare (|
                  ["_1"],
                  Some (M.call_function (|
                    "memoryguard",
                    [
                      [Literal.number 0x80]
                    ]
                  |))
                |)
              )) in
              do! ltac:(M.monadic (
                M.if_ (|
                  M.call_function (|
                    "iszero",
                    [
                      M.call_function (|
                        "lt",
                        [
                          M.call_function (|
                            "calldatasize",
                            []
                          |);
                          [Literal.number 4]
                        ]
                      |)
                    ]
                  |),
                  M.scope (
                    do! ltac:(M.monadic (
                      M.declare (|
                        ["_2"],
                        Some ([Literal.number 0])
                      |)
                    )) in
                    do! ltac:(M.monadic (
                      M.if_ (|
                        M.call_function (|
                          "eq",
                          [
                            [Literal.number 0x26121ff0];
                            M.call_function (|
                              "shr",
                              [
                                [Literal.number 224];
                                M.call_function (|
                                  "calldataload",
                                  [
                                    [Literal.number 0]
                                  ]
                                |)
                              ]
                            |)
                          ]
                        |),
                        M.scope (
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "callvalue",
                                []
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "revert",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0]
                                      ]
                                    |)
                                  |)
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "slt",
                                [
                                  M.call_function (|
                                    "add",
                                    [
                                      M.call_function (|
                                        "calldatasize",
                                        []
                                      |);
                                      M.call_function (|
                                        "not",
                                        [
                                          [Literal.number 3]
                                        ]
                                      |)
                                    ]
                                  |);
                                  [Literal.number 0]
                                ]
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "revert",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0]
                                      ]
                                    |)
                                  |)
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["newFreePtr"],
                              Some (M.call_function (|
                                "add",
                                [
                                  M.get_var (| "_1" |);
                                  [Literal.number 64]
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "or",
                                [
                                  M.call_function (|
                                    "gt",
                                    [
                                      M.get_var (| "newFreePtr" |);
                                      [Literal.number 0xffffffffffffffff]
                                    ]
                                  |);
                                  M.call_function (|
                                    "lt",
                                    [
                                      M.get_var (| "newFreePtr" |);
                                      M.get_var (| "_1" |)
                                    ]
                                  |)
                                ]
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "mstore",
                                      [
                                        [Literal.number 0];
                                        M.call_function (|
                                          "shl",
                                          [
                                            [Literal.number 224];
                                            [Literal.number 0x4e487b71]
                                          ]
                                        |)
                                      ]
                                    |)
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "mstore",
                                      [
                                        [Literal.number 4];
                                        [Literal.number 0x41]
                                      ]
                                    |)
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "revert",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0x24]
                                      ]
                                    |)
                                  |)
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "mstore",
                                [
                                  [Literal.number 64];
                                  M.get_var (| "newFreePtr" |)
                                ]
                              |)
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "mstore",
                                [
                                  M.get_var (| "_1" |);
                                  [Literal.number 4]
                                ]
                              |)
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["_3"],
                              Some (M.call_function (|
                                "add",
                                [
                                  M.get_var (| "_1" |);
                                  [Literal.number 32]
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "mstore",
                                [
                                  M.get_var (| "_3" |);
                                  [Literal.string 0x6162636400000000000000000000000000000000000000000000000000000000]
                                ]
                              |)
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.assign (|
                              ["_2"],
                              Some ([Literal.number 0])
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["_4"],
                              Some (M.call_function (|
                                "extract_byte_array_length",
                                [
                                  M.call_function (|
                                    "sload",
                                    [
                                      [Literal.number 0]
                                    ]
                                  |)
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "gt",
                                [
                                  M.get_var (| "_4" |);
                                  [Literal.number 31]
                                ]
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "mstore",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0]
                                      ]
                                    |)
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.declare (|
                                    ["deleteStart"],
                                    Some ([Literal.number 0x290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e564])
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.assign (|
                                    ["deleteStart"],
                                    Some ([Literal.number 18569430475105882587588266137607568536673111973893317399460219858819262702947])
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.declare (|
                                    ["_5"],
                                    Some (M.call_function (|
                                      "add",
                                      [
                                        M.get_var (| "deleteStart" |);
                                        M.call_function (|
                                          "shr",
                                          [
                                            [Literal.number 5];
                                            M.call_function (|
                                              "add",
                                              [
                                                M.get_var (| "_4" |);
                                                [Literal.number 31]
                                              ]
                                            |)
                                          ]
                                        |)
                                      ]
                                    |))
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.declare (|
                                    ["start"],
                                    Some (M.get_var (| "deleteStart" |))
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  do!
                                    M.scope (
                                      M.pure BlockUnit.Tt
                                    ) in
                                  ltac:(M.monadic (
                                    M.for_ (|
                                      ltac:(M.monadic (
                                        M.call_function (|
                                          "lt",
                                          [
                                            M.get_var (| "start" |);
                                            M.get_var (| "_5" |)
                                          ]
                                        |)
                                      )),
                                      M.scope (
                                        do! ltac:(M.monadic (
                                          M.assign (|
                                            ["start"],
                                            Some (M.call_function (|
                                              "add",
                                              [
                                                M.get_var (| "start" |);
                                                [Literal.number 1]
                                              ]
                                            |))
                                          |)
                                        )) in
                                        M.pure BlockUnit.Tt
                                      ),
                                      M.scope (
                                        do! ltac:(M.monadic (
                                          M.expr_stmt (|
                                            M.call_function (|
                                              "sstore",
                                              [
                                                M.get_var (| "start" |);
                                                [Literal.number 0]
                                              ]
                                            |)
                                          |)
                                        )) in
                                        M.pure BlockUnit.Tt
                                      )
                                    |)
                                  ))
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["srcOffset"],
                              Some ([Literal.number 0])
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.assign (|
                              ["srcOffset"],
                              Some ([Literal.number 32])
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "sstore",
                                [
                                  [Literal.number 0];
                                  M.call_function (|
                                    "or",
                                    [
                                      M.call_function (|
                                        "and",
                                        [
                                          M.call_function (|
                                            "mload",
                                            [
                                              M.get_var (| "_3" |)
                                            ]
                                          |);
                                          M.call_function (|
                                            "shl",
                                            [
                                              [Literal.number 224];
                                              [Literal.number 0xffffffff]
                                            ]
                                          |)
                                        ]
                                      |);
                                      [Literal.number 8]
                                    ]
                                  |)
                                ]
                              |)
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["slot"],
                              Some ([Literal.number 0])
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["offset"],
                              Some ([Literal.number 0])
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["arrayLength"],
                              Some (M.call_function (|
                                "extract_byte_array_length",
                                [
                                  M.call_function (|
                                    "sload",
                                    [
                                      [Literal.number 0]
                                    ]
                                  |)
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.if_ (|
                              M.call_function (|
                                "iszero",
                                [
                                  M.get_var (| "arrayLength" |)
                                ]
                              |),
                              M.scope (
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "mstore",
                                      [
                                        [Literal.number 0];
                                        M.call_function (|
                                          "shl",
                                          [
                                            [Literal.number 224];
                                            [Literal.number 0x4e487b71]
                                          ]
                                        |)
                                      ]
                                    |)
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "mstore",
                                      [
                                        [Literal.number 4];
                                        [Literal.number 0x32]
                                      ]
                                    |)
                                  |)
                                )) in
                                do! ltac:(M.monadic (
                                  M.expr_stmt (|
                                    M.call_function (|
                                      "revert",
                                      [
                                        [Literal.number 0];
                                        [Literal.number 0x24]
                                      ]
                                    |)
                                  |)
                                )) in
                                M.pure BlockUnit.Tt
                              )
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.switch (|
                              M.call_function (|
                                "lt",
                                [
                                  M.get_var (| "arrayLength" |);
                                  [Literal.number 32]
                                ]
                              |),
                              [
                                (
                                  Some (Literal.number 0),
                                  M.scope (
                                    do! ltac:(M.monadic (
                                      M.expr_stmt (|
                                        M.call_function (|
                                          "mstore",
                                          [
                                            [Literal.number 0];
                                            [Literal.number 0]
                                          ]
                                        |)
                                      |)
                                    )) in
                                    do! ltac:(M.monadic (
                                      M.assign (|
                                        ["slot"],
                                        Some ([Literal.number 18569430475105882587588266137607568536673111973893317399460219858819262702947])
                                      |)
                                    )) in
                                    do! ltac:(M.monadic (
                                      M.assign (|
                                        ["offset"],
                                        Some ([Literal.number 31])
                                      |)
                                    )) in
                                    M.pure BlockUnit.Tt
                                  )
                                );
                                (
                                  None,
                                  M.scope (
                                    do! ltac:(M.monadic (
                                      M.assign (|
                                        ["offset"],
                                        Some ([Literal.number 31])
                                      |)
                                    )) in
                                    do! ltac:(M.monadic (
                                      M.assign (|
                                        ["slot"],
                                        Some ([Literal.number 0])
                                      |)
                                    )) in
                                    M.pure BlockUnit.Tt
                                  )
                                )
                              ]
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["value"],
                              Some (M.call_function (|
                                "shl",
                                [
                                  [Literal.number 248];
                                  M.call_function (|
                                    "shr",
                                    [
                                      M.call_function (|
                                        "shl",
                                        [
                                          [Literal.number 3];
                                          M.get_var (| "offset" |)
                                        ]
                                      |);
                                      M.call_function (|
                                        "sload",
                                        [
                                          M.get_var (| "slot" |)
                                        ]
                                      |)
                                    ]
                                  |)
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.declare (|
                              ["memPos"],
                              Some (M.call_function (|
                                "mload",
                                [
                                  [Literal.number 64]
                                ]
                              |))
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "mstore",
                                [
                                  M.get_var (| "memPos" |);
                                  M.call_function (|
                                    "and",
                                    [
                                      M.get_var (| "value" |);
                                      M.call_function (|
                                        "shl",
                                        [
                                          [Literal.number 248];
                                          [Literal.number 255]
                                        ]
                                      |)
                                    ]
                                  |)
                                ]
                              |)
                            |)
                          )) in
                          do! ltac:(M.monadic (
                            M.expr_stmt (|
                              M.call_function (|
                                "return",
                                [
                                  M.get_var (| "memPos" |);
                                  [Literal.number 32]
                                ]
                              |)
                            |)
                          )) in
                          M.pure BlockUnit.Tt
                        )
                      |)
                    )) in
                    M.pure BlockUnit.Tt
                  )
                |)
              )) in
              do! ltac:(M.monadic (
                M.expr_stmt (|
                  M.call_function (|
                    "revert",
                    [
                      [Literal.number 0];
                      [Literal.number 0]
                    ]
                  |)
                |)
              )) in
              M.pure BlockUnit.Tt
            )
          )) in
          M.pure BlockUnit.Tt
        );
    |}.
  End deployed.
End C.

Import Ltac2.

Definition codes : list Code.t :=
  ltac2:(
    let codes := Code.get_codes () in
    exact $codes
  ).
