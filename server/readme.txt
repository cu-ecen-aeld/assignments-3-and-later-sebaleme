### Runtime profiling

## Configuration

The sockettest.sh was modified as follow:
ts=$(date +%s%N)
// Test execution
duration=$SECONDS
echo "Tests complete with success in $((($(date +%s%N) - $ts)/1000000))ms!"


## Results single thread

The single threaded application yields the following results:

Here selected outputs from the /var/log/syslog:
Oct  6 15:17:25 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:61674 after 0.000230 seconds
Oct  6 15:17:27 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:62186 after 0.000136 seconds
Oct  6 15:17:28 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:64234 after 0.000133 seconds
Oct  6 15:17:29 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:64746 after 0.000231 seconds
Oct  6 15:17:30 FE-C-013E3 aesdsocket: Closed connection from 127.0.0.1:2795 after 0.000424 seconds
This makes a total of 1.154ms C-code execution, the rest being the bash script and data transmission

The complete test duration:
Tests complete with success in 5145ms!


## Results multi thread

