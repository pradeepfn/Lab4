echo "astar output "
./cachesim -c 15 -b 5 -s 3 -v 1 -r L -t S < traces/astar.trace
printf "\n\n"
echo "bzip2 output "
./cachesim -c 15 -b 6 -s 5 -v 2 -r N -t B < traces/bzip2.trace
printf "\n\n"
echo "mcf output "
./cachesim -c 15 -b 4 -s 5 -v 2 -r N -t S < traces/mcf.trace
printf "\n\n"
echo "perlbench output "
./cachesim -c 10 -b 4 -s 5 -v 4 -r L -t B < traces/perlbench.trace
