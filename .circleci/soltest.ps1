cd "$PSScriptRoot\.."

.\build\solc\Release\solc.exe --version

mkdir test_results
.\build\test\Release\soltest.exe --color_output=no --show_progress=yes --logger=JUNIT,error,test_results/result.xml -- --no-smt
.\build\test\Release\soltest.exe --color_output=no --show_progress=yes --logger=JUNIT,error,test_results/result_opt.xml -- --optimize --no-smt
