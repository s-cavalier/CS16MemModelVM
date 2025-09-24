make debug

(
    cd kernel;
    make clean;
    make;
)

python kelfparse.py

(
    cd programs/shell;
    make clean;
    make;
)

(
    cd programs/calculator;
    make clean;
    make;
)