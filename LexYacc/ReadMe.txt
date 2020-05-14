一、运行example.cc
    1.example.cc不能使用example.l和example.y运行。
    2.使用example1.l和example1.y运行，执行如下命令：
        yacc -d example1.y
        lex example1.l
        g++ -o example example.cc
        ./example
    3.使用example2.l和example2.y运行，执行如下命令：
        yacc -d example2.y
        lex example2.l
        g++ -o example example.cc
        ./example
