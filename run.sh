
g++ -o build FlightParams.cpp -DCOMMANDS_GENERATOR -I2026_C_AV_CONFMAN/
./build | tee params.txt

python3 send_params.py --port $1 --file params.txt --baud 115200 | tee out.txt

# params-5p.txt : 3088368102
# params-10p.txt : 3515954344
# params-15p.txt : 77783811
