{
  "contracts":
  {
    "test/cmdlineTests/ast_json_import_wrong_evmVersion/input.sol:C": {}
  },
  "sourceList":
  [
    "test/cmdlineTests/ast_json_import_wrong_evmVersion/input.sol"
  ],
  "sources":
  {
    "test/cmdlineTests/ast_json_import_wrong_evmVersion/input.sol":
    {
      "AST":
      {
        "absolutePath": "test/cmdlineTests/ast_json_import_wrong_evmVersion/input.sol",
        "exportedSymbols":
        {
          "C":
          [
            7
          ]
        },
        "id": 8,
        "nodeType": "SourceUnit",
        "nodes":
        [
          {
            "id": 1,
            "literals":
            [
              "solidity",
              ">=",
              "0.0"
            ],
            "nodeType": "PragmaDirective",
            "src": "0:22:0"
          },
          {
            "abstract": false,
            "baseContracts": [],
            "contractDependencies": [],
            "contractKind": "contract",
            "documentation": null,
            "fullyImplemented": true,
            "id": 7,
            "linearizedBaseContracts":
            [
              7
            ],
            "name": "C",
            "nodeType": "ContractDefinition",
            "nodes":
            [
              {
                "body":
                {
                  "id": 5,
                  "nodeType": "Block",
                  "src": "65:21:0",
                  "statements":
                  [
                    {
                      "AST":
                      {
                        "nodeType": "YulBlock",
                        "src": "78:2:0",
                        "statements": []
                      },
                      "evmVersion": "istanbul",
                      "externalReferences": [],
                      "id": 4,
                      "nodeType": "InlineAssembly",
                      "src": "69:11:0"
                    }
                  ]
                },
                "documentation": null,
                "functionSelector": "26121ff0",
                "id": 6,
                "implemented": true,
                "kind": "function",
                "modifiers": [],
                "name": "f",
                "nodeType": "FunctionDefinition",
                "overrides": null,
                "parameters":
                {
                  "id": 2,
                  "nodeType": "ParameterList",
                  "parameters": [],
                  "src": "50:2:0"
                },
                "returnParameters":
                {
                  "id": 3,
                  "nodeType": "ParameterList",
                  "parameters": [],
                  "src": "65:0:0"
                },
                "scope": 7,
                "src": "40:46:0",
                "stateMutability": "pure",
                "virtual": false,
                "visibility": "public"
              }
            ],
            "scope": 8,
            "src": "23:65:0"
          }
        ],
        "src": "0:89:0"
      }
    }
  },
  "version": "0.6.2-develop.2020.1.14+commit.e8556fa1.mod.Linux.g++"
}
