mv out/rom.bin out/rom.bin.bak
sudo docker run --rm -v $PWD:/m68k -t registry.gitlab.com/doragasu/docker-sgdk:v1.70
./run.sh
