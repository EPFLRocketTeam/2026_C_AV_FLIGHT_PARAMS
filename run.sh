
g++ -o build FlightParams.cpp -DCOMMANDS_GENERATOR -I2026_C_AV_CONFMAN/
./build | tee params.txt

python3 send_params.py --port $1 --file params.txt --baud 115200 | tee out.txt
